# 可玩 UE 示例

使用 UE 5.8.2（CL 56702186），安装匹配的 Visual Studio 2022 C++ 工具链与 Windows SDK。右键 `GhostRevealExample.uproject` 生成工程文件，编译 `GhostRevealExampleEditor`（Development Editor / Win64），打开工程后在默认地图 `FlashlightRoom` 点击 Play。

WASD 移动，鼠标瞄准，F 开关手电筒，Esc 释放鼠标，点击重新捕获。扫过 Manny 观察表面局部显形，绕过短墙观察挡光。实际灯源与相机共点同轴，手电筒筒体单独偏置到右下角。

示例运行逻辑位于 `Source/GhostRevealExample`，通过插件公开接口连接玩家、灯源和目标。插件不依赖示例类或人形资产；接入已有项目只需复制 `Plugins/FlashlightGhostReveal`，参考[接入说明](Plugins/FlashlightGhostReveal/README.md)。

仓库仅包含可编辑工程，不附带打包演示、二进制、资产生成工具或自动测试入口。Manny、动画及骨架预览依赖来自 UE 模板；必要的 Quinn 资源属于骨架预览依赖，不是额外演示。来源及条款见[第三方说明](../THIRD_PARTY_NOTICES.md)。
