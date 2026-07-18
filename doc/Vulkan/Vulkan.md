Vulkan 是一种新一代**图形和计算 API**，可为各种设备（从 PC 和游戏机 到 手机和嵌入式平台）中使用的现代 GPU 提供高效、跨平台的访问。

Vulkan 不是一种语言，而是一种允许开发人员以跨平台和跨供应商的方式**对其现代 GPU 硬件进行编程**的方式。Khronos Group 创建并维护 Vulkan。

核心上，Vulkan 是一个[API 规范](https://registry.khronos.org/vulkan/#apispecs)，符合标准的硬件实现都遵循该规范。公共规范是从 [./xml/vk.xml](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/xml/vk.xml) Vulkan 注册表文件生成的，该文件位于 [Vulkan-Doc](https://github.com/KhronosGroup/Vulkan-Docs) 的官方公共 Vulkan 规范仓库中。还提供了 [XML 模式](https://registry.khronos.org/vulkan/specs/latest/registry.html)的文档。

Khronos Group 与 Vulkan 规范一起发布了从 [API 注册表](https://registry.khronos.org/vulkan/#apiregistry)生成的 [C99](https://www.open-std.org/jtc1/sc22/wg14/www/standards) [头文件](https://github.com/KhronosGroup/Vulkan-Headers/tree/main/include/vulkan)，开发人员可以使用这些文件与 Vulkan API 进行交互。

Khronos Group 还有另一个标准 [OpenGL](https://www.khronos.org/opengl/)，它也是一个3D图形API，Vulkan不是OpenGL的直接替代品，而是一个显式API，允许对GPU进行更显式的控制。

---

![what_is_vulkan_compared_to_gl.png](D:\develop\BrushEngine\doc\Vulkan\img\what_is_vulkan_compared_to_gl.png)

Vulkan 的三大核心优势：**高性能（Performance）、执行可预测（Predictability）、跨平台（Portability）**

- **OpenGL/OpenGL ES**：高层驱动抽象，大量逻辑交由显卡驱动隐式处理，不同厂商驱动行为差异大，易产生额外开销。

- **Vulkan**：轻量化薄驱动（Thin Driver），将底层控制权交给应用层，消除 “驱动黑盒逻辑”，运行行为统一、可预测，跨设备兼容性更强。

架构分层对比（从上至下：应用层 → 中间层 → 驱动 / GPU 层）:

- 线程与命令管理：
  
  - OpenGL / OpenGL ES
    
    - 线程模型：**单线程上下文（Single thread per context）**，渲染命令基本只能在单一线程提交，无法利用 CPU 多核并行能力，多核 CPU 资源浪费严重。
    
    - 命令生成：无独立命令缓冲区设计，绘制指令即时提交，驱动需实时解析、校验。
  
  - Vulkan
    
    - 原生**多线程架构**：支持多线程并行录制命令缓冲区（Multi-threaded generation of command buffers），多核 CPU 可分工生成渲染指令，充分释放算力。
    
    - 多队列支持：提供图形队列、通用命令队列、DMA 传输队列等多类硬件队列，任务可并行执行，硬件利用率更高。

- 内存与资源管理：
  
  - OpenGL / OpenGL ES
    
    - 内存分配、资源生命周期**完全由驱动隐式管理**，应用无法干预分配策略，容易出现内存碎片、分配效率不可控的问题，开发者无法做精细化优化。
  
  - Vulkan
    
    - 采用**显式内存分配**：API 对象（缓冲区、纹理、管线等）必须由应用提前显式创建、分配内存后再使用。
      
      优势：内存使用完全可控，分配逻辑贴合业务场景，执行效率与内存占用高度可预测。

- 同步机制：
  
  - OpenGL / OpenGL ES
    
    - **隐式同步**：资源依赖、GPU-CPU 同步全部由驱动自动处理，开发者无感知。驱动为保证渲染安全，会增加大量等待与校验逻辑，带来额外 CPU 开销。
  
  - Vulkan
    
    - **显式同步（Explicit Synchronization）**：同步逻辑由应用手动控制（信号量、栅栏、管线屏障等）。驱动不再做冗余等待，CPU 开销大幅降低，代价是开发者需要自行梳理任务依赖关系。

- 驱动设计（核心差异）：
  
  - OpenGL / OpenGL ES
    
    - 驱动复杂臃肿：内置完整的 GLSL 预处理器、编译器、全局状态机、运行时错误检测逻辑。
    
    - 痛点：不同厂商驱动实现逻辑不同，同一代码在不同设备上行为不一致，开发者需要适配各类厂商的驱动特性，调试难度大。
    
    - 常驻式错误处理：**始终开启运行时错误检查**，即使正式发布版本也会持续产生性能损耗。
  
  - Vulkan
    
    - 极简薄驱动（Thin Driver）：剥离了着色器编译、全局状态管理、默认错误校验等冗余逻辑，驱动仅做硬件指令转发，实现简单、各厂商行为高度统一。
    
    - 分层调试机制：调试 / 验证层（Validation layers）**按需加载**，仅开发阶段启用，正式版本可完全关闭，零额外开销。
    
    - 无 “驱动魔法”：所有硬件控制逻辑暴露给应用，应用可结合自身业务做全局深度优化。

- 着色器体系：
  
  - OpenGL / OpenGL ES
    
    - 着色器语言：原生使用 **GLSL**，驱动内置完整 GLSL 编译器与预处理器。
    
    - 缺陷：运行时动态编译着色器，启动与切换耗时高；不同驱动对 GLSL 语法、扩展的支持存在差异。
  
  - Vulkan
    
    - 中间标准：强制使用 **SPIR-V** 二进制中间着色器语言。
    
    - 优势：
      
      1. 着色器可**预编译**为 SPIR-V 离线文件，运行时无需编译，启动速度更快；
      2. 语言中立：前端可对接 GLSL、HLSL 等多种高级着色器语言，灵活性极强；
      3. 统一标准，彻底解决不同设备着色器兼容性问题。

- API 控制模式：
  
  - OpenGL / OpenGL ES
    
    - **高层 GPU 控制（High-level Driver Abstraction）**：
      
      全局状态机模型，渲染状态（混合、裁剪、视口等）为全局变量，频繁切换状态会触发驱动大量校验，是主要性能瓶颈之一。
  
  - Vulkan
    
    - **分层显式 GPU 控制（Layered / Explicit GPU Control）**：
      
      无全局状态，所有渲染状态封装为管线对象（PSO），提前创建固化；API 对象必须**显式创建后再使用**，运行时仅执行指令提交，流程极简、性能稳定。

**OpenGL / OpenGL ES**：分两套 API，桌面端为 OpenGL，移动端为 OpenGL ES，接口存在差异，跨平台需要额外适配。

**Vulkan**：**桌面、移动端统一单套 API**，一套代码可无缝跑在 PC、手机、主机等设备上，跨平台能力大幅提升。

---

在开发流程中使用一些较轻的库来帮助抽象 Vulkan 中一些更繁琐的方面是很常见的。这里有一些 [库](https://github.com/KhronosGroup/Khronosdotorg/blob/main/api/vulkan/resources.md#libraries) 来 [帮助开发](https://github.com/vinjn/awesome-vulkan#libraries)。

Vulkan 是开发人员创建硬件加速应用程序的工具。Vulkan 可用于开发许多用例的应用程序。虽然 Vulkan 应用程序可以选择使用下面描述的功能的子集，但它的设计目标是使开发人员可以在单个 API 中使用所有这些功能。Vulkan 是一个工具箱，你可用多种方法来完成一项任务。

- **图形**：2D 和 3D 图形主要是 Vulkan API 设计的目的。Vulkan 的设计目标是允许开发人员创建硬件加速的图形应用程序。
  
  所有 Vulkan 实现都必须支持图形，但WSI系统不是必需的。

- **计算**：由于 GPU 的并行特性，一种称为 [GPGPU](https://en.wikipedia.org/wiki/General-purpose_computing_on_graphics_processing_units) 的新型编程风格可以被用来利用 GPU 进行计算任务。Vulkan 支持 `VkQueues`、`VkPipelines` 等计算变体，这些变体允许将 Vulkan 用于通用计算。*所有 Vulkan 实现都必须支持计算。*

- **光线追踪**：光线追踪是一种替代渲染技术，它基于模拟光的物理行为的概念。
  
  在 1.2.162 规范中，跨供应商的 API 光线追踪支持已作为一组扩展添加到 Vulkan 中。这些主要是 [`VK_KHR_ray_tracing_pipeline`](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#VK_KHR_ray_tracing_pipeline)、[`VK_KHR_ray_query`](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#VK_KHR_ray_query) 和 [`VK_KHR_acceleration_structure`](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#VK_KHR_acceleration_structure)。

- **视频**：通过 [Vulkan 视频扩展](https://www.khronos.org/blog/khronos-finalizes-vulkan-video-extensions-for-accelerated-h.264-and-h.265-decode)，开发人员可以实时使用硬件加速的视频解码功能。该功能通过 [VK_KHR_video_queue](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_video_queue.html)、[VK_KHR_video_decode_queue](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_video_decode_queue.html)、[VK_KHR_video_decode_h264](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_video_decode_h264.html) 和 [VK_KHR_video_decode_h265](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_KHR_video_decode_h265.html) 扩展公开。Vulkan 视频遵循为应用程序提供对视频处理调度、同步和内存利用的灵活、细粒度的控制的理念。

- **安全关键**：Vulkan SC（“安全关键”）旨在将现代 GPU 的图形和计算功能引入汽车、航空电子、工业和医疗领域的安全关键系统。它于[2022 年 3 月 1 日公开发布](https://www.khronos.org/news/press/khronos-releases-vulkan-safety-critical-1.0-specification-to-deliver-safety-critical-graphics-compute)，并且该规范可在此处[获取](https://www.khronos.org/vulkansc/)。*Vulkan SC 基于 Vulkan 1.2，但删除了安全关键市场不需要的功能，通过消除忽略的参数和未定义的行为来提高规范的鲁棒性，并支持增强的运行时故障检测、报告和纠正。*

---

Vulkan 规范（通常称为*Vulkan Spec*）是对 Vulkan API 如何工作的官方描述，最终用于决定什么是有效的 Vulkan 用法，什么不是。乍一看，Vulkan 规范似乎是一个非常庞大且枯燥的文本块，但它通常是开发时最有用的打开项。

- Vulkan 规范变体：
  
  可以为任何版本和任何扩展组合构建 Vulkan 规范。Khronos Group 托管 [Vulkan 规范注册表](https://registry.khronos.org/vulkan/specs/)，其中包含一些大多数开发人员会觉得足够的公开可用变体。任何人都可以从 [Vulkan-Docs](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/BUILD.adoc) 构建自己的 Vulkan 规范变体。
  
  构建 Vulkan 规范时，您需要传入要构建的 Vulkan 版本以及要包含的扩展。没有任何扩展的 Vulkan 规范也称为[核心版本](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-coreversions)，因为它是实现为了符合标准需要支持的最小 Vulkan 量。

- Vulkan 规范版本：
  
  Vulkan 1.0 到 1.3，有规范的专门版本。为了减少构建排列，从 Vulkan 1.4 开始有一个 `latest` 版本，它将始终更新到最新的 Vulkan 版本。
  
  [Vulkan SDK](https://vulkan.lunarg.com/doc/sdk/1.4.304.0/windows/1.4-extensions/vkspec.html) 将始终包含创建时使用的规范版本。

- Vulkan 规范格式：
  
  Vulkan 规范可以构建为不同的格式。
  
  - Antora
  
  - HTML
  
  - PDF
  
  - Man页面

---

虽然 Vulkan 在许多平台上运行，但每个平台在 Vulkan 的管理方式上都有细微的差异。

Windows 7、Windows 8、Windows 10 和 Windows 11 支持 Vulkan。获取 [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) 并运行 [vulkaninfo.exe](https://vulkan.lunarg.com/doc/sdk/latest/windows/vulkaninfo.html) 可执行文件，以轻松检查 Vulkan 支持以及设备提供的所有功能。

Vulkan 需要 **Vulkan 加载器** 和 **Vulkan 驱动程序**（也称为 *Vulkan 实现*）。驱动程序负责将 Vulkan API 调用转换为有效的 Vulkan 实现。最常见的情况是 GPU 硬件供应商发布一个驱动程序，用于在物理 GPU 上运行 Vulkan。应该注意的是，有可能拥有一个完全基于软件的 Vulkan 实现，尽管性能影响会非常明显。

在检查 Vulkan 支持时，区分**平台支持**和**设备支持**之间的差异非常重要。有平台支持并不意味着有设备支持。对于设备支持，需要确保有完全实现 Vulkan 的 Vulkan 驱动程序可用。Vulkan 驱动程序有几种不同的变体。

针对物理 GPU 硬件的驱动程序是 Vulkan 实现中最常见的情况。虽然某些 GPU 可能具有运行 Vulkan 的物理功能，但仍然需要驱动程序来控制它。驱动程序负责以最有效的方式将 Vulkan 调用映射到硬件。

与任何软件一样，驱动程序也会更新，这意味着同一物理设备和平台可能存在许多不同的驱动程序。有一个由 Sascha Willems 开发和维护的 [Vulkan 数据库](https://vulkan.gpuinfo.org/)，它是记录的 Vulkan 实现细节的最大集合。仅仅因为物理设备或平台不在 Vulkan 数据库中，并不意味着它不存在。

“空驱动”一词指的是任何接受 Vulkan API 调用但不执行任何操作的驱动程序。这通常用于测试与驱动程序的交互，而无需任何实际的工作实现。许多用例，例如为新功能创建 CTS 测试、[测试验证层](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/creating_tests.md#running-tests-on-devsim-and-mockicd)等，都依赖于空驱动的概念。

Khronos 提供了 [Mock ICD](https://github.com/KhronosGroup/Vulkan-Tools/tree/master/icd) 作为一种在各种平台上工作的空驱动的实现。

可以创建一个仅在 CPU 上运行的 Vulkan 实现。如果需要测试与硬件无关的 Vulkan，这非常有用，但与空驱动不同，它也会输出有效的结果。[SwiftShader](https://github.com/google/swiftshader) 是基于 CPU 实现的一个示例。

**VIA (Vulkan 安装分析器)**：

[Vulkan SDK](https://vulkan.lunarg.com/sdk/home) 中包含一个用于检查计算机上 Vulkan 安装的实用工具。它在 Windows、Linux 和 macOS 上受支持。VIA 可以

- 确定系统上 Vulkan 组件的状态

- 验证您的 Vulkan 加载器和驱动程序是否已正确安装

- 捕获您的系统状态，以便在提交错误时用作附件

有关更多信息，请查看 [关于 VIA 的 SDK 文档](https://vulkan.lunarg.com/doc/sdk/latest/windows/via.html)。

跨平台检查 Vulkan 支持的一种简单方法是创建一个简单的 “Hello World” Vulkan 应用程序。`vkCreateInstance` 函数用于创建 Vulkan 实例，也是编写有效 Vulkan 应用程序的最短方法。Vulkan SDK 提供了一个最小的 [vkCreateInstance](https://vulkan.lunarg.com/doc/view/latest/windows/tutorial/html/01-init_instance.html) 示例 `01-init_instance.cpp`，可以使用它。

---

Vulkan 使用[主版本号、次版本号、补丁版本号](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-coreversions-versionnumbers)的版本控制系统。目前，Vulkan 有 3 个次版本发布（1.0、1.1、1.2 和 1.3），它们彼此**向后兼容**。应用程序可以使用[vkEnumerateInstanceVersion](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#vkEnumerateInstanceVersion) 来检查支持的 Vulkan 实例版本。LunarG 还有一个关于如何查询和检查支持版本的[白皮书](https://www.lunarg.com/wp-content/uploads/2019/02/Vulkan-1.1-Compatibility-Statement_01_19.pdf)。在跨次版本工作时，需要注意一些细微之处。

重要的是要记住实例级版本和设备级版本之间存在差异。加载器和实现可能支持不同的版本。Vulkan 规范中的[查询版本支持](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-coreversions-queryingversionsupport)部分详细介绍了如何查询实例和设备级别支持的版本。

对于所有 Vulkan 主要版本，只有一个受支持的头文件。这意味着不存在“Vulkan 1.0 头文件”之类的东西，因为次版本和补丁版本的所有头文件都是统一的。这不应与生成 Vulkan 规范的 1.0 版本的能力混淆，因为**同一补丁版本的 Vulkan 规范和头文件将匹配**。比如生成的 1.0.42 Vulkan 规范将与 1.x.42 头文件匹配。

Vulkan SDK 有许多版本，这些版本映射到它将打包的头文件版本。

在 Vulkan 的次版本之间，[一些扩展](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#versions-1.1)被[提升](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-compatibility-promotions)到[核心版本](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-coreversions)。当以较新的 Vulkan 次版本为目标时，应用程序无需在实例和设备创建时启用新提升的扩展。但是，如果应用程序想要保持向后兼容性，则需要启用这些扩展。

结构体和枚举取决于正在使用的头文件，而不是查询的实例或设备的版本。例如，结构体 `VkPhysicalDeviceFeatures2` 在 Vulkan 1.1 发布之前曾是 `VkPhysicalDeviceFeatures2KHR`。无论使用哪个 1.x 版本的 Vulkan，应用程序都应在其代码中使用 `VkPhysicalDeviceFeatures2`，因为它与最新的头文件版本匹配。对于代码中确实有 `VkPhysicalDeviceFeatures2KHR` 的应用程序，无需担心，因为 Vulkan 头文件还会别名任何提升的结构体和枚举（`typedef VkPhysicalDeviceFeatures2 VkPhysicalDeviceFeatures2KHR;`）。

使用较新命名的原因是 Vulkan 规范本身将仅引用 `VkPhysicalDeviceFeatures2`，而无论生成的是哪个版本的 Vulkan 规范。使用较新的命名可以更容易地快速搜索到该结构体的使用位置。

由于函数用于与加载器和实现进行交互，因此在次版本之间工作时需要更加小心。

在次要版本之间，可能会添加、删除、使某些功能位变为可选或强制。所有已更改的功能的详细信息都将在[核心修订](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#versions)部分中描述。Vulkan规范中的[功能要求](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#features-requirements)部分可用于查看在次要版本中实现所需的特性列表。

目前，所有Vulkan版本都共享相同的最小/最大限制要求，但任何更改都将在Vulkan规范的[限制要求](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#limits-minmax)部分列出。

每个Vulkan的次要版本都映射到[必须支持的SPIR-V版本](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#spirvenv)。

- Vulkan 1.0 支持 SPIR-V 1.0

- Vulkan 1.1 支持 SPIR-V 1.3 及以下版本

- Vulkan 1.2 支持 SPIR-V 1.5 及以下版本

- Vulkan 1.3 支持 SPIR-V 1.6 及以下版本

应用程序有责任确保 `VkShaderModule` 中的 SPIR-V 版本对于相应的 Vulkan 版本是有效的。

---

Vulkan 的每个小版本都[提升](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-compatibility-promotion)了一组不同的扩展到核心。这意味着如果应用程序请求至少该 Vulkan 版本（前提是该版本受实现支持），则不再需要启用扩展来使用其功能。

[SPIR-V](https://registry.khronos.org/SPIR-V/) 是图形着色器阶段和计算内核的二进制中间表示。使用 Vulkan，应用程序仍然可以使用**高级着色语言**（如 **GLSL** 或 **HLSL**）编写着色器，但在使用 [vkCreateShaderModule](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#vkCreateShaderModule) 时需要 SPIR-V 二进制文件。Khronos 有一篇非常好的关于 SPIR-V 及其优点的 [白皮书](https://registry.khronos.org/SPIR-V/papers/WhitePaper.pdf)，以及对该表示形式的高级描述。

Vulkan 有一个完整的章节定义了 [Vulkan 如何与 SPIR-V 着色器进行交互](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#interfaces)。大多数与 SPIR-V 交互的有效用法发生在着色器编译在一起的管线创建过程中。

SPIR-V 有许多功能，因为它还有 Vulkan 以外的其他目标。要查看 Vulkan 需要支持的功能，可以参考 [附录](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#spirvenv-capabilities)。Vulkan 中的某些扩展和功能只是为了检查是否支持某些 SPIR-V 功能。

![what_is_spirv_dxc.png](D:\develop\BrushEngine\doc\Vulkan\img\what_is_spirv_dxc.png)

[DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) 支持 [将 HLSL 转换为 SPIR-V](https://github.com/Microsoft/DirectXShaderCompiler/wiki/SPIR%E2%80%90V-CodeGen)。

**DXC（DirectX Shader Compiler）** 编译 HLSL 着色器的双分支工作流程，分别面向 DirectX 12 与 Vulkan 两大图形 API。DXC 可将同一份 HLSL 代码，经统一前端处理后，分别编译为适配 DirectX 12 的 DXIL 和适配 Vulkan 的 SPIR-V 两种主流着色器中间格式，并各自对应专属工具集。

- **输入源**：以 **HLSL**（微软高级着色语言）作为原始着色器代码输入。

- **通用前端阶段**：DXC 先完成**前端解析与校验**，并生成**AST（抽象语法树）**，这是整个编译流程的统一前置环节。

- **两条后端编译分支**
  
  - **DirectX 12 分支**：通过 **DXIL 后端**输出 **DXIL**（DirectX 专属中间字节码），搭配**DXIL Tools**工具链使用，服务于 DirectX 12 图形接口。
  - **Vulkan 分支**：通过 **SPIR-V 后端** 输出 **SPIR-V**（Vulkan 标准中间表示格式），配套**SPIR-V Tools**工具链，用于 Vulkan 图形接口。

---

**SPIR-V**与**SPIRV-Cross**在多平台着色器语言、图形 API 之间的编译与转译链路：

![what_is_spirv_spriv_cross.png](D:\develop\BrushEngine\doc\Vulkan\img\what_is_spirv_spriv_cross.png)

- **源码编译环节**：以**HLSL**这类主流高级着色器语言为输入，借助`DirectXShaderCompiler`（DXC）分别完成两种编译：一是生成适配 Direct3D 生态的**DXIL**，对接 D3D 9/10/11、D3D12；二是生成通用中间格式**SPIR-V**，作为跨平台转换的核心枢纽。

- **SPIR-V 转译分发环节**：**SPIRV-Cross**承担 SPIR-V 的逆向转译工作，搭配不同专用编译器，可将 SPIR-V 分别转为**GLSL、ESSL、MSL、HLSL**等着色器语言，分别对应适配 Vulkan、OpenGL、OpenGL ES、Metal、DirectX 系列等主流图形 API 与平台。

SPIR-V 是标准化的可移植着色器中间字节码，实现不同高级着色语言的统一中转；SPIRV-Cross 则是关键转译工具，打通了 SPIR-V 与各类平台专属着色语言的双向转换能力，是跨平台图形开发的重要组件。

[Vulkan SDK](https://vulkan.lunarg.com/doc/sdk/latest/windows/spirv_toolchain.html)概述了为开发人员构建和打包的所有 SPIR-V 工具。

---

Vulkan 生态系统由许多用于开发的工具组成。

Khronos 托管了 [Vulkan 示例](https://github.com/KhronosGroup/Vulkan-Samples)，这是一个代码和教程的集合，演示了 API 的用法并解释了性能最佳实践的实现。

LunarG 开发和维护 Vulkan 生态系统组件，并且目前是 [Vulkan 加载器](https://github.com/KhronosGroup/Vulkan-Loader) 和 [Vulkan 验证层](https://github.com/KhronosGroup/Vulkan-ValidationLayers) Khronos Group 存储库的管理者。 此外，LunarG 还提供 [Vulkan SDK](https://vulkan.lunarg.com/) 并开发其他关键工具，例如 [Vulkan 配置器](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) 和 [GFXReconstruct](https://vulkan.lunarg.com/doc/sdk/latest/windows/capture_tools.html)。

Vulkan 层是增强 Vulkan 系统的可选组件。 它们可以拦截、评估和修改从应用程序到硬件的现有 Vulkan 函数。 层以库的形式实现，可以使用 [Vulkan 配置器](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) 启用和配置。

[`VK_LAYER_KHRONOS_validation`](https://docs.vulkan.net.cn/guide/latest/validation_overview.html#khronos-validation-layer)，Khronos 验证层。它是每个开发人员调试 Vulkan 应用程序时的第一道防线。验证层包含多个功能：

- [同步验证](https://vulkan.lunarg.com/doc/sdk/latest/windows/synchronization_usage.html)：识别由于在读取或写入相同内存区域的操作（绘制、复制、调度、位块传输）之间缺少或不正确的同步操作而导致的资源访问冲突。

- [GPU 辅助验证](https://vulkan.lunarg.com/doc/sdk/latest/windows/gpu_validation.html)：对着色器代码进行检测，以执行在着色器执行期间产生的错误条件的运行时检查。

- [着色器 printf](https://vulkan.lunarg.com/doc/sdk/latest/windows/debug_printf.html)：通过将任何感兴趣的值“打印”到调试回调或 stdout 来调试着色器代码。

- [最佳实践警告](https://vulkan.lunarg.com/doc/sdk/latest/windows/best_practices.html)：突出显示潜在的性能问题、有问题的用法模式、常见错误。

除了 Khronos 层之外，Vulkan SDK 还包含其他有用的平台独立层：

- [`VK_LAYER_LUNARG_api_dump`](https://vulkan.lunarg.com/doc/sdk/latest/windows/api_dump_layer.html)，一个用于记录 Vulkan API 调用的层。 API 转储层将 API 调用、参数和值打印到已标识的输出流。

- [`VK_LAYER_LUNARG_gfxreconstruct`](https://vulkan.lunarg.com/doc/sdk/latest/windows/capture_tools.html)，一个用于捕获使用 Vulkan 创建的帧的层。 此层是 GFXReconstruct 的一部分，GFXReconstruct 是一种用于捕获和重放 Vulkan API 调用的软件。

- [`VK_LAYER_LUNARG_device_simulation`](https://vulkan.lunarg.com/doc/sdk/latest/windows/device_simulation_layer.html)，一个用于测试 Vulkan 应用程序可移植性的层。 设备模拟层可用于测试 Vulkan 应用程序是否可以在功能较低的 Vulkan 设备上运行。

- [`VK_LAYER_LUNARG_screenshot`](https://vulkan.lunarg.com/doc/sdk/latest/windows/screenshot_layer.html)，一个屏幕截图层。 将 Vulkan 应用程序的渲染图像捕获为可查看的图像。

- [`VK_LAYER_LUNARG_monitor`](https://vulkan.lunarg.com/doc/sdk/latest/windows/monitor_layer.html)，一个帧率监视器层。 在窗口标题栏中显示 Vulkan 应用程序的 FPS，以提示性能。

[`Vulkan 扩展层`](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/)是一个实现并非在所有地方都可用的扩展的层集合。 默认情况下，如果底层驱动程序提供了扩展，则这些层将自行禁用。

调试在 GPU 上运行的内容可能非常困难，幸运的是，有一些工具可以提供帮助。

- [AMD Radeon GPU 分析器 (RGA)](https://gpuopen.cn/rga/)

- [AMD Radeon 光线追踪分析器 (RRA)](https://gpuopen.cn/radeon-raytracing-analyzer/)

- [Arm 图形分析器](https://developer.arm.com/Tools%20and%20Software/Graphics%20Analyzer)

- [NVIDIA Nsight](https://developer.nvidia.com/nsight-graphics)

- [PVRCarbon](https://developer.imaginationtech.com/)

- [RenderDoc](https://renderdoc.org/)

- [GFXReconstruct](https://vulkan.lunarg.com/doc/sdk/latest/windows/capture_tools.html)

以下工具可帮助调试崩溃。

- [AMD GPU 检测器](https://gpuopen.cn/radeon-gpu-detective)

- [NVIDIA Nsight Aftermath SDK](https://developer.nvidia.com/nsight-aftermath)

对于与 GPU 相关的任何内容，最好不要假设，并且尽可能进行性能分析。 以下是已知的分析器列表，可帮助您进行开发。

- [AMD Radeon GPU 性能分析器](https://gpuopen.cn/rgp/) - 用于 AMD Radeon GPU 的低级性能分析工具。

- [Intel® GPA](https://www.intel.com/content/www/us/en/developer/tools/graphics-performance-analyzers/overview.html) - Intel 的图形性能分析器，支持捕获和分析 Vulkan 应用的多帧流。

- [NVIDIA Nsight](https://developer.nvidia.com/nsight-graphics)

- [OCAT](https://github.com/GPUOpen-Tools/OCAT) - 开源捕获和分析工具 (OCAT) 为 D3D11、D3D12 和 Vulkan 提供 FPS 覆盖和性能测量。

- [PVRTune](https://developer.imaginationtech.com/)

- [VKtracer](https://www.vktracer.com/) - 跨供应商和跨平台的性能分析器。

---

**有效使用（VU）** 在 [Vulkan 规范](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#fundamentals-validusage) 中被明确定义为 为了在应用程序中实现明确定义的运行时行为而**必须**满足的一组条件。

Vulkan 作为显式 API 的主要优点之一是，实现（驱动程序）不会浪费时间检查有效输入。在 OpenGL 中，实现必须始终检查有效使用情况，这会增加明显的开销。Vulkan 中没有 [glGetError](https://www.khronos.org/opengl/wiki/OpenGL_Error) 等效项。

有效使用情况将在规范中列在每个函数和结构之后。例如，如果 VUID 在 `VkBindImageMemory` 中检查无效的 `VkImage`，则规范中的有效使用情况会在 `VkBindImageMemory` 下找到。这是因为验证层只会在应用程序执行期间的 `VkBindImageMemory` 处知道所有信息。

当应用程序根据规范中的有效使用情况提供无效输入时，结果是*未定义行为*。在这种状态下，Vulkan 不做任何保证，因为未定义行为可能会导致任何事情。**虽然未定义行为可能在一种实现中似乎有效，但在另一种实现中很有可能会失败。**

有效使用ID（VUID）：`VUID` 是为每个有效使用情况赋予的唯一 ID。这允许一种在规范中轻松指向有效使用情况的方法。

由于 Vulkan 不进行任何错误检查，因此在开发时立即启用 [验证层](https://github.com/KhronosGroup/Vulkan-ValidationLayers) 以帮助捕获无效行为**非常重要**。应用程序也不应将验证层与其应用程序一起发布，因为它们会明显降低性能并且是为开发阶段设计的。Khronos 验证层过去由多个层组成，但现在已统一为单个 `VK_LAYER_KHRONOS_validation` 层。[LunarG 的白皮书](https://www.lunarg.com/wp-content/uploads/2019/04/UberLayer_V3.pdf) 中解释了更多详细信息。

验证层在不断更新和改进，因此始终可以获取源代码并[自行构建](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/BUILD.md)。如果您需要预构建版本，则所有受支持的平台都有多种选择：**Windows** - [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) 附带构建的验证层，以及有关如何在 [Windows](https://vulkan.lunarg.com/doc/sdk/latest/windows/validation_layers.html) 上使用它们的说明。

验证层会在发生错误时尝试提供尽可能多的有用信息。每个错误消息都包含统一的日志记录模式，这允许在任何错误中轻松找到信息。模式如下：

- 日志状态（例如，`Error:`、`Warning:` 等）

- VUID

- 涉及的对象数组
  
  - 数组的索引
  
  - Dispatch Handle 值
  
  - 可选名称
  
  - 对象类型

- 发生错误的函数或结构体

- 层创建的用于帮助描述问题的消息

- 规范中的完整有效使用

- 有效使用的链接

---

**Vulkan 概念术语与其他 API 中所用术语之间的映射**

如果您正在搜索您所知道的 API 中使用的概念的 Vulkan 等效项，您可以在此列表中找到您所知道的术语，然后在 Vulkan 规范 中搜索相应的 Vulkan 术语。并非所有内容都完全匹配，只是为了粗略地了解从哪里开始在规范中查找。

| **Vulkan**        | **GL,GLES**                                                                                                                                                                    | **DirectX**                           | **Metal**                                      |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------- | ---------------------------------------------- |
| 缓冲区设备地址           |                                                                                                                                                                                | GPU 虚拟地址                              |                                                |
| 缓冲区视图、纹素缓冲区       | 纹理缓冲区                                                                                                                                                                          | 类型化缓冲区 SRV，类型化缓冲区 UAV                 | 纹理缓冲区                                          |
| 颜色附件              | 颜色附件                                                                                                                                                                           | 渲染目标                                  | 颜色附件或渲染目标                                      |
| 命令缓冲区             | 上下文的一部分，显示列表，NV_command_list                                                                                                                                                   | 命令列表                                  | 命令缓冲区                                          |
| 命令池               | 上下文的一部分                                                                                                                                                                        | 命令分配器                                 | 命令队列                                           |
| 条件渲染              | 条件渲染                                                                                                                                                                           | 预测                                    |                                                |
| 协同矩阵              |                                                                                                                                                                                | 波矩阵                                   | SIMD 组矩阵                                       |
| 深度/模板附件           | 深度附件和模板附件                                                                                                                                                                      | 深度/模板视图                               | 深度附件和模板附件，深度渲染目标和模板渲染目标                        |
| 描述符               |                                                                                                                                                                                | 描述符                                   | 参数                                             |
| 描述符池              |                                                                                                                                                                                | 描述符堆                                  | 堆                                              |
| 描述符集              |                                                                                                                                                                                | 描述符表                                  | 参数缓冲区                                          |
| 描述符集布局绑定，推送描述符    |                                                                                                                                                                                | 根参数                                   | 着色器参数列表中的参数                                    |
| 设备组               | 隐式（例如 SLI、CrossFire）                                                                                                                                                           | 多适配器设备                                | 对等组                                            |
| 设备内存              |                                                                                                                                                                                | 堆                                     | 放置堆                                            |
| 事件                |                                                                                                                                                                                | 拆分屏障                                  |                                                |
| 栅栏                | 栅栏、同步                                                                                                                                                                          | `ID3D12Fence::SetEventOnCompletion`   | 完成处理程序，`-[MTLCommandBuffer waitUntilComplete]` |
| 片段着色器             | 片段着色器                                                                                                                                                                          | 像素着色器                                 | 片段着色器或片段函数                                     |
| 片段着色器互锁           | [GL_ARB_fragment_shader_interlock](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_fragment_shader_interlock.txt)                                                       | 光栅化器顺序视图 (ROV)                        | 光栅顺序组                                          |
| 帧缓冲区              | 帧缓冲区对象                                                                                                                                                                         | 资源集合                                  | MTLRenderPassDescriptor                        |
| 堆                 |                                                                                                                                                                                | ID3D12Heap                            | MTLHeap                                        |
| 图像                | 纹理和渲染缓冲区                                                                                                                                                                       | 纹理                                    | 纹理                                             |
| 图像布局              |                                                                                                                                                                                | 资源状态                                  |                                                |
| 图像平铺              |                                                                                                                                                                                | 图像布局、颜色分量重排                           |                                                |
| 图像视图              | 纹理视图                                                                                                                                                                           | 渲染目标视图、深度/模板视图、着色器资源视图、无序访问视图         | 纹理视图                                           |
| 接口匹配 (`in`/`out`) | 可变 ([在 GLSL 4.20 中移除](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.20.pdf))                                                                                      | 匹配语义                                  | 每个顶点的输入属性                                      |
| 调用                | 调用                                                                                                                                                                             | 线程、通道                                 | 线程、通道                                          |
| 层                 |                                                                                                                                                                                | 切片                                    | 切片                                             |
| 逻辑设备              | 上下文                                                                                                                                                                            | 设备                                    | 设备                                             |
| 内存类型              | 自动管理，[纹理存储提示](https://registry.khronos.org/OpenGL/extensions/APPLE/APPLE_texture_range.txt)，[缓冲区存储](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_buffer_storage.txt) | 堆类型，CPU 页面属性                          | 存储模式，CPU 缓存模式                                  |
| 多视图渲染             | 多视图渲染                                                                                                                                                                          | 视图实例化                                 | 顶点放大                                           |
| 物理设备              |                                                                                                                                                                                | 适配器，节点                                | 设备                                             |
| 管线                | 状态和程序或程序管线                                                                                                                                                                     | 管线状态                                  | 管线状态                                           |
| 管线屏障，内存屏障         | 纹理屏障，内存屏障                                                                                                                                                                      | 资源屏障                                  | 纹理屏障，内存屏障                                      |
| 管线布局              |                                                                                                                                                                                | 根签名                                   |                                                |
| 队列                | 上下文的一部分                                                                                                                                                                        | 命令队列                                  | 命令队列                                           |
| 信号量               | 栅栏、同步                                                                                                                                                                          | 栅栏                                    | 栅栏，事件                                          |
| 着色器模块             | 着色器对象                                                                                                                                                                          | 从 `D3DCompileFromFile` 生成的 `ID3DBlob` | 着色器库                                           |
| 着色率附件             |                                                                                                                                                                                | 着色率图像                                 | 光栅化率图                                          |
| 稀疏块               | 稀疏块                                                                                                                                                                            | 瓦片                                    | 稀疏瓦片                                           |
| 稀疏图像              | 稀疏纹理                                                                                                                                                                           | 保留资源 (D12)，平铺资源 (D11)                 | 稀疏纹理                                           |
| 存储缓冲区             | 着色器存储缓冲区                                                                                                                                                                       | 原始或结构化缓冲区 UAV                         | `device` 地址空间中的缓冲区                             |
| 子群                | 子群                                                                                                                                                                             | 波                                     | SIMD组, 四元组                                     |
| 表面                | HDC, GLXDrawable, EGLSurface                                                                                                                                                   | 窗口                                    | 层                                              |
| 交换链               | HDC, GLXDrawable, EGLSurface 的一部分                                                                                                                                              | 交换链                                   | 层                                              |
| 交换链图像             | 默认帧缓冲                                                                                                                                                                          |                                       | 可绘制纹理                                          |
| 任务着色器             |                                                                                                                                                                                | 放大着色器                                 | 对象着色器                                          |
| 细分控制着色器           | 细分控制着色器                                                                                                                                                                        | 外壳着色器                                 | 细分计算内核                                         |
| 细分评估着色器           | 细分评估着色器                                                                                                                                                                        | 域着色器                                  | 细分后顶点着色器                                       |
| 时间线信号量            |                                                                                                                                                                                | D3D12栅栏                               | 事件                                             |
| 变换反馈              | 变换反馈                                                                                                                                                                           | 流输出                                   |                                                |
| 统一缓冲区             | 统一缓冲区                                                                                                                                                                          | 常量缓冲区视图 (CBV)                         | 位于 `constant` 地址空间的缓冲区                         |
| 工作组               | 工作组                                                                                                                                                                            | 线程组                                   | 线程组                                            |

---

加载器负责将应用程序映射到 Vulkan 层和 Vulkan 可安装客户端驱动程序 (ICD)。

任何人都可以创建自己的 Vulkan 加载器，只要他们遵循 [加载器接口](https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderInterfaceArchitecture.md)。可以构建 [参考加载器](https://github.com/KhronosGroup/Vulkan-Loader/blob/main/BUILD.md)，也可以从 [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) 获取选定平台的构建版本。

**链接到加载器**

[Vulkan 头文件](https://github.com/KhronosGroup/Vulkan-Headers)仅提供 Vulkan 函数原型。在构建 Vulkan 应用程序时，必须将其链接到加载器，否则会出现未定义 Vulkan 函数引用的错误。链接加载器有两种方式：[直接链接](https://github.com/KhronosGroup/Vulkan-Loader/blob/main/loader/LoaderAndLayerInterface.md#directly-linking-to-the-loader)和[间接链接](https://github.com/KhronosGroup/Vulkan-Loader/blob/main/loader/LoaderAndLayerInterface.md#indirectly-linking-to-the-loader)，这不应与“静态和动态链接”混淆。

- [直接链接](https://github.com/KhronosGroup/Vulkan-Loader/blob/main/loader/LoaderAndLayerInterface.md#directly-linking-to-the-loader)在编译时
  
  - 这需要有一个构建的 Vulkan 加载器（作为静态或动态库），你的构建系统可以找到它。
  
  - 构建系统（Visual Studio、CMake 等）有关于如何链接到库的文档。尝试在线搜索“(插入构建系统) 链接到外部库”。

- [间接链接](https://github.com/KhronosGroup/Vulkan-Loader/blob/main/loader/LoaderAndLayerInterface.md#indirectly-linking-to-the-loader)在运行时
  
  - 使用动态符号查找（通过诸如 `dlsym` 和 `dlopen` 之类的系统调用），应用程序可以初始化自己的分发表。这允许应用程序在找不到加载器时优雅地失败。它还为应用程序调用 Vulkan 函数提供了最快的方式。
  
  - [Volk](https://github.com/zeux/volk/) 是一个开源的元加载器实现，旨在简化此过程。

每个平台都可以设置自己的规则来强制执行 Vulkan 加载器。

Windows：

[Vulkan SDK](https://vulkan.lunarg.com/sdk/home) 为 Windows 提供了一个预构建的加载器。

[Vulkan SDK](https://vulkan.lunarg.com/doc/sdk/latest/windows/getting_started.html) 中的“入门”页面解释了如何在 Windows 上找到加载器。
