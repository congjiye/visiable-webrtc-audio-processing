# WebRTC 音频处理

使用 WebRtc 对音频进行处理，可以播放处理前后的音频，显示处理前后的音频波形图

## 安装

- WebRtc 音频处理可执行程序编译安装
- 可视化 web 页面安装

## 可执行程序安装

```bash
# 在根目录下执行如下命令
mkdir build
cd build
cmake ..
make
```

将编译好的可执行文件复制到 app 目录下。

```bash
# windows
cp audio_processing.exe app

# linux
cp audio_processing app
```

在终端下执行 `audio_processing.exe -h` 或 `./audio_processing -h` 查看是否可以使用。

## 可视化 web 页面安装

进入 app 目录下执行 `pip install -r requirements.txt`

执行 `python main.py` 后打开浏览器输入 `localhost:5050` 查看页面

## TODO

- [ ] VAD 切分音频
- [ ] 回声消除