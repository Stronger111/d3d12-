1:游戏引擎架构,Windows引擎启动,日志和断言系统,Windows平台层,Linux平台层,应用层和入口函数,内存子系统,事件系统(各个系统进行通信)。
2:输入系统 
  渲染子系统
    a:渲染前端
      Meshs，Textures,Materials, Render Passes,Render Graph
    b:渲染后端
      Renderer APIs Vulkan OpenGL  GPU upload GPU上传 DPaw Calls 
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
-困难的部分是思考内存和数据结构,以及写的逻辑如何改变数据和如何操作内存
-引擎多种描述符问题 纹理数组
-着色器反射系统
-批处理
 PUSHD 更换当前目录
- KOHI快捷键
  ESC 应用退出

# 数学
 * 三维空间中点到直线的最短距离(几何意义:叉积的模长代表平行四边形的面积,除于底边长度(方向向量模长)得到高,即点到直线的垂直距离)

Renderer:
- renderable texture support
- batch rendering
- JobSystem
- 字体渲染 字库->图集  代码点->codePoint 1 2 3 4 字节  对象拾取 原理 pass 绘制 物体ID->rgba 取出鼠标位置 然后读取纹理 然后rgba 在转换为 物体ID 
- 光追 PSO 多线程 hism
- Gizmo系統
- 地形分块和剔除(Culling) 
  地形LOD skirt_side 平滑LOD过渡  四叉树进行LOD OTree 地块动态加载和卸载 
  线框shader pipeline
  Shader 热重载
- 阴影
    Shadow Map -> Cascade Shadow Map 
       1:阴影消失的地方 柔和过渡 shadow_distance shadow_fade_distance 阴影淡出
- PBR
  Phone光照模型 PBR ->Albedo Normal Metalle Roughness AO IBL 间接漫反射 ->Roughness AO Metallic合并一张
  R通道 金属度 G通道 粗糙度 B通道 Ao
- Intel VTune Profiler 性能分析工具
- 插件形式添加模块代码
- Mac Os Molten VK
- 描述符
  根据更新频率对描述符集进行分组  Shader和描述符池一起使用
  OpenGL 不必处理管理描述符以及手动同步内存之类的事情
- 控制台

  RenderGraph LoadResource 加载完Graph资源 在加载具体 Pass内部的资源
# 引擎待完成
# Math
  Alt+41420 根号 Alt+178 平方号  1:44:25
  
-bug
-"Vulkan buffer creation failed for object shader  out_error_local_memory  windows ？？ TODO  重启解决问题 后面需要查一下
- 赋值内存错误,内存空间开辟不足和内存不正确
- 更详细的Vulkan Debug信息
- attribute=vec4,in_tangent vulkan 属性布局 一定要和VBO 数据对应 不对应会出现花屏 比如切线为vec4 传进去为vec3 管线Vertex input stride步幅
- 传入UBO 数据不正确
- Vulkan DrawCall 不可以绑定多余的Pipeline 不然会导致上一个DrawCall绘制错误。
- 矩阵乘法顺序错误。
- 传入采样器12个 但是shader里面越界访问也会报错
- 环境变量更改需要重启
- filesystem_read_all_text 文件 
- Debug root->children,10 打印数组的10个元素. ,6 可以打印数组元素
- 模版测试-裁剪输入框外部的内容 模版需要clear
- 可视化Debug 以及特殊情况 来验证计算的正确性
- Duke bug fix  state_ptr->handle.hwnd 没有赋值 导致 VK_ERROR_SURFACE_LOST_KHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR 方法报错
  //修复 https://stackoverflow.com/questions/66912703/vulkan-vkgetphysicaldevicesurfacecapabilitieskhr-returning-vk-error-surface-los
  //vkCmdBeginRendering 报错地址0XFFFFFFFFFFFFFFFFFFFFFF 原因创建Pipeline 管线没有创建VkPipelineRenderingCreateInfo导致没有填充完整信息报错.    VkPipelineRenderingCreateInfoKHR 是VkPipelineRenderingCreateInfo扩展版本 支持vulkan1.2版本 

# 工具
  * Rendoc Pro 
    https://www.renderdoc.net/download/
  vscode c++格式化指针对齐
    https://blog.csdn.net/xiaxinkai/article/details/113705651  Ctrl 加,打开
  
# 流程

-Feature特性
 CSG系统
-JobSystem
 --Ring queue 循环队列 -> CPU调度工作
 --多线程从另一个线程调用另一个 会产生共享资源竞争 Semaphore
-空闲列表
 --内存管理 
 --内存地址对齐 Memory Alignment Allocator updates
-即时模式和保留模式 GUI
-场景管理
 -深度优先遍历物体
-地形系统
  地形分块和LOD 使用Compute Shader进行计算和粒子
-多视口系统ViewPort
-Gizmo系统->数学 射线和平面 AABB,OBB求交检测拾取对象,交互鼠标事件
-Unity程序化绘制 可以获取ComputeBuffer数据 无需顶点Buffer和VBO绑定数据 以及DrawCall数据 无法专递顶点色 法线 切线等数据  

The audio plugin requires an installatiion of OpenAL.
 - Linux: use a package manager to install OpenAL, if not already installed (i.e. `sudo apt install openal` for Ubuntu or `sudo pacman -S openal` on Arch)
 - macOS: install openal-soft via homebrew: `brew install openal-soft`. Note on M1 macs this installs to `/opt/homebrew/opt/openal-soft/`, where the `include`, `lib`, and `'bin` directories can be found. The `build-all.sh` script accounts for this version of the install.