1:游戏引擎架构,Windows引擎启动,日志和断言系统,Windows平台层,Linux平台层,应用层和入口函数,内存子系统,事件系统(各个系统进行通信)。
2:输入系统 
  渲染子系统
    a:渲染前端
      Meshs，Textures,Materials, Render Passes,Render Graph
    b:渲染后端
      Renderer APIs Vulkan OpenGL  GPU upload GPU上传 Draw Calls 
      Vulkan Extensions, Validation Layers, Debugger 
      Vulkan 逻辑设备和队列
      同步对象和信号量  vulkan FrameBuffer
3:快速编译 https://gnuwin32.sourceforge.net/packages/make.htm
4:数学工具 paint.net  
5:-Werror=vla 禁止可变数组 int[index_cout] 是不被允许
6:从硬盘加载图片,纹理系统,材质系统,几何系统,资源系统  
7:动画
8:适配器模式适配库
9:game/editor logic library (dll/.so) hot reload    -------- kvars(configurable 'global' settings)  --- engline configuration  ---- timeline system  --- skeletal animation sysytem 
-KSM 文件格式从Obj转换  和引擎数据格式高度吻合
-可写纹理不会自动释放 引用计数为0 不会释放
-批处理
 PUSHD 更换当前目录

Renderer:
- renderable texture support
- batch rendering
- JobSystem
- 字体渲染 字库->图集  代码点->codePoint 1 2 3 4 字节  对象拾取 原理 pass 绘制 物体ID->rgba 取出鼠标位置 然后读取纹理 然后rgba 在转换为 物体ID 
- 光追 pso 多线程 hism

-bug
-"Vulkan buffer creation failed for object shader  out_error_local_memory  windows ？？ TODO  重启解决问题 后面需要查一下
- 赋值内存错误,内存空间开辟不足和内存不正确
- 更详细的Vulkan Debug信息

-Feature
-JobSystem
 --Ring queue 循环队列 -> CPU调度工作
-空闲列表
 --内存管理 
 --内存地址对齐 Memory Alignment Allocator updates
-即时模式和保留模式 GUI
-场景管理
 -深度优先遍历物体