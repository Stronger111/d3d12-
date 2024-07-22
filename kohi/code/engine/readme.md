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
-

Renderer:
- renderable texture support
- batch rendering