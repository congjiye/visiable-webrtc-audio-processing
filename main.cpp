#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

#define DR_WAV_IMPLEMENTATION

#include "aecm.h"
#include "agc.h"
#include "dr_wav.h"
#include "ns.h"
#include "vad.h"

#include "spdlog/spdlog.h"
#include "cmdline.h"

using namespace std;

int16_t *read_wave_file_s16(char *filename, uint32_t *sample_rate, uint64_t *sample_count, unsigned int *channel) {
    int16_t *buffer = drwav_open_and_read_file_s16(filename, channel, sample_rate, sample_count);
    if (buffer == nullptr) {
        SPDLOG_ERROR("Read wave file failed");
    }
    return buffer;
}

bool write_wave_file_s16(char *filename, int16_t *buffer,
                         size_t sample_rate, size_t sample_count,
                         unsigned int channel) {
    drwav_data_format format = {};
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_PCM;
    format.channels = channel;
    format.sampleRate = (drwav_uint32) sample_rate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, sample_count, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != sample_count) {
            SPDLOG_ERROR("Write wave file failed");
            return false;
        }
    }
    return true;
}

struct AGCInputConfig {
    int32_t in_mic_level;
    int32_t out_mic_level;
    uint8_t saturation_warning;
};

int AGC(int16_t *buffer, uint32_t sample_rate,
        size_t sample_count, int16_t mode,
        WebRtcAgcConfig &config,
        AGCInputConfig &inputConfig) {
    auto *handle = (LegacyAgc *) WebRtcAgc_Create();
    if (handle == nullptr) {
        SPDLOG_ERROR("Create agc handle failed");
        return EXIT_FAILURE;
    }

    if (WebRtcAgc_Init(handle, inputConfig.in_mic_level, inputConfig.out_mic_level, mode, sample_rate) != 0) {
        SPDLOG_ERROR("Initialize AGC handle failed");
        WebRtcAgc_Free(handle);
        return EXIT_FAILURE;
    }

    if (WebRtcAgc_set_config(handle, config) != 0) {
        SPDLOG_ERROR("Set AGC config failed");
        WebRtcAgc_Free(handle);
        return EXIT_FAILURE;
    }

    size_t samples = sample_rate / 100;
    size_t total = sample_count / samples;
    int16_t out_buffer[320];
    int16_t *out16 = out_buffer;
    int16_t *input = buffer;
    for (int i = 0; i < total; ++i) {
        if (WebRtcAgc_Process(handle, (const int16_t *const *) &input, 1, samples, (int16_t *const *) &out16,
                              inputConfig.in_mic_level, &inputConfig.out_mic_level, 0,
                              &inputConfig.saturation_warning) != 0) {
            SPDLOG_ERROR("WebRtcAgc_Processing failed");
            WebRtcAgc_Free(handle);
            return EXIT_FAILURE;
        }
        memcpy(input, out_buffer, samples * sizeof(int16_t));
        input += samples;
    }

    const size_t remain_samples = sample_count - total * samples;
    if (remain_samples > 0) {
        if (total > 0) {
            input = input - samples + remain_samples;
        }
        if (WebRtcAgc_Process(handle, (const int16_t *const *) &input, 1, samples, (int16_t *const *) &out16,
                              inputConfig.in_mic_level, &inputConfig.out_mic_level, 0,
                              &inputConfig.saturation_warning) != 0) {
            SPDLOG_ERROR("Failed in WebRtcAgc_Process during filtering the last chunk");
            WebRtcAgc_Free(handle);
            return EXIT_FAILURE;
        }
        memcpy(&input[samples - remain_samples], &out_buffer[samples - remain_samples],
               remain_samples * sizeof(int16_t));
        input += samples;
    }
    WebRtcAgc_Free(handle);
    return EXIT_SUCCESS;
}

int NS(int16_t *buffer, uint32_t sample_rate, uint64_t sample_count, uint32_t channel, int level) {
    NsHandle *handle = WebRtcNs_Create();
    if (handle == nullptr) {
        SPDLOG_ERROR("Create NS handle failed");
        return EXIT_FAILURE;
    }

    if (WebRtcNs_Init(handle, sample_rate) != 0) {
        SPDLOG_ERROR("Initialize NS handle failed");
        WebRtcNs_Free(handle);
        return EXIT_FAILURE;
    }

    if (WebRtcNs_set_policy(handle, level) != 0) {
        SPDLOG_ERROR("Set NS policy failed");
        WebRtcNs_Free(handle);
        return EXIT_FAILURE;
    }
    size_t samples = sample_rate / 100;
    int16_t *input = buffer;
    int16_t *frame_buffer = (int16_t *) malloc(sizeof(*frame_buffer) * samples);
    size_t total = sample_count / samples;
    for (int i = 0; i < total; ++i) {
        for (int j = 0; j < samples; ++j) {
            frame_buffer[j] = input[j];
        }
        int16_t *ns_in[1] = {frame_buffer};
        int16_t *ns_out[1] = {frame_buffer};
        WebRtcNs_Analyze(handle, ns_in[0]);
        WebRtcNs_Process(handle, (const int16_t *const *) ns_in, 1, ns_out);
        for (int j = 0; j < samples; ++j) {
            input[j] = frame_buffer[j];
        }
        input += samples;
    }
    WebRtcNs_Free(handle);
    free(frame_buffer);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%@] %v");
    cmdline::parser a;
    a.add<string>("input", 'i', "input file path", true, "");
    a.add<string>("output", 'o', "output file path", true, "");
    a.add<bool>("use-agc", '\0', "whether do agc", false, true);
    a.add<int>("agc-level", '\0', "AGC level [0-3] "
                                  "[kAgcModeUnchanged, kAgcModeAdaptiveAnalog, kAgcModeAdaptiveDigital, kAgcModeFixedDigital]",
               false, 2);
    a.add<int>("in-mic", '\0', "in mic level", false, 0);
    a.add<int>("out-mic", '\0', "out mic level", false, 255);
    a.add<uint8_t>("saturation-warning", '\0', "A returned value of 1 indicates a saturation event\n"
                                               "has occurred and the volume cannot be further\n"
                                               "reduced. Otherwise will be set to 0.", false, 1);
    a.add<int16_t>("compress-gain", '\0', "compress gain db", false, 9);
    a.add<int16_t>("target-level", '\0', "target level db", false, 3);
    a.add<uint8_t>("limiter-enable", '\0', "limiter enable", false, 1);
    a.add<bool>("use-ns", '\0', "whether do noise suppression", false, true);
    a.add<int>("ns-level", '\0', "NS level [0-3] [kLow, kModerate, kHigh, kVeryHigh]",
               false, 1);
    a.parse_check(argc, argv);

    string input_file_path = a.get<string>("input");
    if (access(input_file_path.c_str(), F_OK) == -1) {
        SPDLOG_ERROR("File does not exists");
        return EXIT_FAILURE;
    }

    uint32_t sample_rate = 0;
    uint64_t sample_count = 0;
    unsigned int channel = 0;
    int16_t *buffer = read_wave_file_s16(const_cast<char *>(input_file_path.c_str()), &sample_rate, &sample_count,
                                         &channel);

    if (buffer == nullptr) {
        SPDLOG_ERROR("File is empty.Try to check your file.");
        free(buffer);
        return EXIT_FAILURE;
    }

    if (sample_rate != 16000 || channel != 1) {
        SPDLOG_ERROR("Audio file should be 16k and single channel");
        free(buffer);
        return EXIT_FAILURE;
    }

    if (a.get<bool>("use-ns")) {
        SPDLOG_INFO("NS level is {}", a.get<int>("ns-level"));
        if (NS(buffer, sample_rate, sample_count, channel, a.get<int>("ns-level")) != 0) {
            SPDLOG_ERROR("Failed in processing noise suppression");
            free(buffer);
            return EXIT_FAILURE;
        }
        SPDLOG_INFO("Done");
    }

    if (a.get<bool>("use-agc")) {
        AGCInputConfig inputConfig = {a.get<int>("in-mic"),
                                      a.get<int>("out-mic"),
                                      a.get<uint8_t>("saturation-warning")};
        WebRtcAgcConfig config = {a.get<int16_t>("target-level"),
                                  a.get<int16_t>("compress-gain"),
                                  a.get<uint8_t>("limiter-enable")};
        SPDLOG_INFO("AGC level is {}", a.get<int>("agc-level"));
        SPDLOG_INFO("AGC input config in-mic-level={} out-mic-level={} saturation-warning={}",
                    inputConfig.in_mic_level, inputConfig.out_mic_level, inputConfig.saturation_warning);
        SPDLOG_INFO("WebRtcAgc target-level={} compress-gain={} limiter-enable={}",
                    config.targetLevelDbfs, config.compressionGaindB, config.limiterEnable);
        if (AGC(buffer, sample_rate, sample_count, a.get<int>("agc-level"), config, inputConfig) != 0) {
            SPDLOG_ERROR("Failed in processing agc module");
            free(buffer);
            return EXIT_FAILURE;
        }
        SPDLOG_INFO("Done");
    }

    string output_file_path = a.get<string>("output");
    if (!write_wave_file_s16(const_cast<char *>(output_file_path.c_str()), buffer, sample_rate, sample_count,
                             channel)) {
        SPDLOG_ERROR("Save wave file failed");
        free(buffer);
        return EXIT_FAILURE;
    }

    free(buffer);
    return EXIT_SUCCESS;
}
