# Detection Server (DetSvr)


## TODO-LIST
- [x] cpp-httplib服务搭建
- [x] logger配置
- [x] 代码重构
- [x] post + json 发送数据
- [x] 落盘jpg影像测试 
- [x] base64解码
- [x] nolohmann json格式序列化、反序列化实现
- [x] config配置文件 json
- [x] 动态插件
- [x] 修改detectionservice类，一次加载detector，构造函数新增serviceconfig
- [x] 修改Logger为单例模式
- [x] 插件配置项
- [x] 算法开发者框架
- [x] git版本管理（客户端与服务端开发）
- [x] 添加license授权
- [x] opencv读取图像 
- [] 日志功能使用文件还是std::cout，应当考虑与linux的systemctl服务和jounalctl，应当能够定期清理
- [] tensorrt推理，串流程
- [x] 开机自启动
- [] 部署脚本
- [] 视频流的实时性
- [x] rtsp输入
- [] rtmp输入
- [] csi输入
- [] 视频文件输入
- [] http-base64输入
- [] http-flv输入
- [] rtsp输出
- [] rtsp-server输出
- [] videofile输入
- [x] 屏幕输出