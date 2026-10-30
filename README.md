# Flashlight Ghost Reveal

UE 5.8.2 手电筒局部显形插件：光锥内的人物表面显形，墙体阻挡光源时保持隐藏。

- 核心插件：`Example/Plugins/FlashlightGhostReveal`，包含 Runtime C++ 源码和正式材质。
- 可编辑示例：`Example/GhostRevealExample.uproject`，包含第一人称手电筒、Manny 待机目标和可绕行墙体。
- [插件接入说明](Example/Plugins/FlashlightGhostReveal/README.md) · [示例说明](Example/README.md) · [第三方资产说明](THIRD_PARTY_NOTICES.md)

安装 UE 5.8.2 和匹配的 Visual Studio 2022 C++ 工具链、Windows SDK，生成工程文件并编译 `GhostRevealExampleEditor`（Development Editor / Win64），打开工程后点击 Play。仓库不包含预编译插件、打包程序或缓存。

渲染方式为 Deferred + 光源空间深度捕获 + Unlit Masked 材质，不修改引擎。当前面向单主视图、单灯、普通静态/骨骼网格及 Opaque/Masked 遮挡物。多灯、真实半透明和 Nanite 目标不在保证范围；TSR 边缘及关灯后可能有少量时域残留。没有当前完整效果的正式性能预算结论。

公开仓库不代表原创代码已授予开源许可；本仓库暂未指定原创代码的开源许可证。Epic 资产按第三方说明中的条款使用。
