from logging import error
from typing import Optional, Union
from fastapi import FastAPI, File, UploadFile, Form, Request, status, HTTPException
from fastapi.encoders import jsonable_encoder
from fastapi.exceptions import RequestValidationError
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel
from loguru import logger
from starlette.responses import HTMLResponse, Response
from starlette.templating import Jinja2Templates
import uvicorn
import shutil
import uuid
import time
import sys
import os

app = FastAPI()
app.mount('/static', StaticFiles(directory='static'), name='static')
templates = Jinja2Templates(directory='templates')

# 配置跨域
origins = ["http://localhost"]
app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

platform = sys.platform


# 设置日志保存位置
log_dir_path = os.path.join(os.path.dirname(__file__), 'logs')
if not os.path.exists(log_dir_path):
    os.mkdir(log_dir_path)
log_file_path = os.path.join(log_dir_path, f'{time.strftime("%Y-%m-%d")}.log')


# rotation: 设置生成的文件时间
# retention: 设置删除日志文件时间
# enqueue: 线程安全
# diagnose: 显示详细信息
logger.add(sink=log_file_path,
           format='{time} - {level} - {message}',
           rotation="12:00", retention="5 days",
           level="INFO", enqueue=True, diagnose=True)


# 保存文件目录
save_file_dir = os.path.join(os.path.dirname(__file__), 'static', 'audios')
if not os.path.exists(save_file_dir):
    os.mkdir(save_file_dir)


@app.exception_handler(RequestValidationError)
async def request_validation_exception_handler(request: Request, err: RequestValidationError):
    logger.warning(
        f'Invalid arguments {request.method} {request.url} {err.errors()}')
    return JSONResponse(
        status_code=status.HTTP_400_BAD_REQUEST,
        content=jsonable_encoder({"detail": err.errors()})
    )


# AGC 配置
class AGCConfig(BaseModel):
    in_mic: Optional[int] = 0
    out_mic: Optional[int] = 255
    agc_level: Optional[int] = 2
    saturation_warning: Optional[int] = 1
    compose_gain: Optional[int] = 9
    target_level: Optional[int] = 3
    limiter_enable: Optional[int] = 1


# NS 配置
class NSConfig(BaseModel):
    ns_level: Optional[int] = 1


# Audio Processing 配置
class APConfig:

    def __init__(self, use_agc, in_mic, out_mic, agc_level, saturation_warning,
                 compose_gain, target_level, limiter_enable, use_ns, ns_level) -> None:
        self.use_agc = use_agc
        self.in_mic = in_mic
        self.out_mic = out_mic
        self.agc_level = agc_level
        self.saturation_warning = saturation_warning
        self.compose_gain = compose_gain
        self.target_level = target_level
        self.limiter_enable = limiter_enable
        self.use_ns = use_ns
        self.ns_level = ns_level


def gen_ap_command(file_path: str, dst_path: str, config: APConfig) -> str:
    cmd = f""" -i {file_path} -o {dst_path}
        --use-agc={config.use_agc} --agc-level={config.agc_level} 
        --in-mic={config.in_mic} --out-mic={config.out_mic} 
        --saturation-warning={config.saturation_warning} 
        --compress-gain={config.compose_gain} 
        --target-level={config.target_level} 
        --limiter-enable={config.limiter_enable} 
        --use-ns={config.use_ns} --ns-level={config.ns_level}"""
    if platform == 'win32':
        cmd = 'audio_processing.exe' + cmd
        return cmd
    cmd = './audio_processing' + cmd
    return cmd


def success_process_response(*, data: Union[list, dict, str]) -> Response:
    return JSONResponse(
        status_code=status.HTTP_200_OK,
        content={
            'code': 200,
            'data': data
        }
    )


def error_process_response(*, data: str = None, message: str = "Internal Error") -> Response:
    return JSONResponse(
        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
        content={
            'code': 500,
            'message': message,
            'data': data
        }
    )


def gen_wave_image(origin_path: str, processed_path: str):
    origin_image_path = f'static/images/{str(uuid.uuid4())}.png'
    processed_image_path = f'static/images/{str(uuid.uuid4())}.png'
    os.system(
        f'ffmpeg -i {origin_path} -filter_complex "showwavespic=s=640*120" -frames:v 1 {origin_image_path} -loglevel quiet -y')
    os.system(
        f'ffmpeg -i {processed_path} -filter_complex "showwavespic=s=640*120" -frames:v 1 {processed_image_path} -loglevel quiet -y')
    return origin_image_path, processed_image_path


@app.get('/')
async def index(request: Request):
    return templates.TemplateResponse(name='index.html', context={'request': request})


@app.post('/upload')
async def process_upload_audio(request: Request,
                               use_agc: bool = Form(...),
                               in_mic: Optional[int] = 0,
                               out_mic: Optional[int] = 255,
                               agc_level: Optional[int] = 2,
                               saturation_warning: Optional[int] = 1,
                               compose_gain: Optional[int] = 9,
                               target_level: Optional[int] = 3,
                               limiter_enable: Optional[int] = 1,
                               use_ns: bool = Form(...),
                               ns_level: Optional[int] = 1,
                               file: UploadFile = File(...)):
    file_path = f'static/audios/{str(uuid.uuid4())}{file.filename.split(".")[-1]}'
    transfer_path = f'static/audios/{str(uuid.uuid4())}.wav'
    logger.info(file_path)
    logger.info(f'Save upload file into {file_path}')

    data = await file.read()
    with open(file_path, 'wb') as f:
        f.write(data)

    logger.info('Finished saving upload file')
    if os.system(f'ffmpeg -f s16le -ar 16000 -ac 1 -i {file_path} {transfer_path} -loglevel quiet -y') != 0:
        os.remove(file_path)
        logger.warning(f'Transfer audio file {file.filename} to 16k 1c failed')
        return error_process_response(data={
            'cause': 'transfer audio by ffmpeg failed'
        })

    config = APConfig(use_agc, in_mic, out_mic, agc_level, saturation_warning,
                      compose_gain, target_level, limiter_enable, use_ns, ns_level)

    dst_path = f'static/audios/{str(uuid.uuid4())}.wav'
    cmd = gen_ap_command(transfer_path, dst_path, config)
    if os.system(cmd) != 0:
        logger.warning(f'{file.filename} do audio processing failed')
        return {error_process_response(data={
            'cause': 'audio processing failed'
        })}

    os.remove(transfer_path)
    origin_wave_image, processed_wave_image = gen_wave_image(
        file_path, dst_path)
    return success_process_response(data={
        'origin_audio': file_path,
        'processed_audio': dst_path,
        'origin_wave_image': origin_wave_image,
        'processed_wave_image': processed_wave_image
    })


if __name__ == '__main__':
    uvicorn.run("main:app", host="0.0.0.0", port=5000, reload=True)
