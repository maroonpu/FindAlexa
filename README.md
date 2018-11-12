FindAlexa
==================


### 依赖
##### [nghttp2-1.25.0](http://www.nghttp2.org/)<br />
##### [libcurl-7.55.1](https://curl.haxx.se/)<br />
##### [portaudio](http://www.portaudio.com/)<br />
##### [Rapidjson](http://rapidjson.org/)<br />
#### 注意：必须先安装nghttp2后再安装libcurl

### 使用
##### 0.修改makefile中的相应路径，make编译。
##### 1.运行scripts/init.py，初始化环境。
##### 2.根据上一步运行结果提示修改相应文件。
##### 3.执行main文件，等待10s左右完成初始化。
##### 4.输入 ‘b’ 开始录音，自动返回结果并播放。

### 注意
##### 请将openwrt_widora源码与本代码放在同一目录下
