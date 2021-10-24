# UAV-Video
该项目为基于无人机视频的船舶检测，输入为视频文件，并进行循环读取。（该项目使用插件方式）

- 在服务器端训练yolov5-m模型
- 在服务器端，运行gen_wts.py文件，将.pt文件转化为.wts文件（具体参照yolov5/README.md）
- 将.wts文件拷贝至detsrv/build的目录下，重命名为uavship.wts
- 在Jetson终端上使用CMake编译项目，生成libmyplugin.so共享库和yolov5可执行文件
- 使用yolov5生成.engine文件：`./yolov5 -s uavship.wts uavship.engine m`