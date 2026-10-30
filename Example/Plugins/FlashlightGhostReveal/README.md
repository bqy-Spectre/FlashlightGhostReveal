# 手电筒局部显形：接入说明

最终效果始终包含光源深度遮挡：鬼魂默认隐藏，只有手电筒光锥内、且灯到表面之间没有挡光物的部分显形。相机遮挡由正常深度测试处理。

## 安装

将本仓库 `Example/Plugins/FlashlightGhostReveal` 复制到接收工程的 `Plugins` 文件夹，生成工程文件并使用 UE 正常编译，启用插件后重启编辑器。内容浏览器中启用“显示插件内容”。

源码版本针对 Windows、UE 5.8.2（CL 56702186），不附带预编译二进制。需要安装匹配 UE 的 Visual Studio 2022 C++ 工具链与 Windows SDK；仅蓝图工程也需要可编译 C++ 插件的工具链。插件不自动替换 GameMode、输入配置或渲染设置。

## 蓝图接入已有游戏

1. 在手电筒 Actor 上添加 `Flashlight Reveal Depth Component`。BeginPlay 调用其 `Initialize`，传入现有 `SpotLightComponent`；检查返回值。组件内部会建立灯源快照组件和更新依赖。
2. 在鬼魂 Actor 上添加 `Ghost Reveal Component`。默认自动绑定该 Actor 的所有 Mesh 材质槽；如果只需处理部分 Mesh，关闭 `Bind Owner Meshes`，逐个调用 `Bind Target`。
3. 在初始化后调用灯源深度组件的 `Register Target`，传入鬼魂组件。返回 false 表示引用无效或不在同一 World。所有已注册目标从光源捕获中排除。
4. 开关时调用深度组件的 `Set Reveal Enabled`，并由游戏控制 Spot Light 的可见性。也可以直接使用插件的 `Flashlight Reveal Actor`，其 `Set Flashlight Enabled` 同时控制两者。
5. 将手电筒自身模型等不应挡光的 Primitive 加入 `Additional Hidden Components`。不要排除真正的场景墙体。

第一人称接入要使光圈圆心与十字线一致：将实际 Spot Light 或插件灯源 Actor 附到相机，设置相对位置为零、相对旋转为零。右下角握持偏移只施加到手电筒 Mesh。这样光束与显形共享相机射线，前后景都使用同一个屏幕光圈；示例相机关闭 Bloom。不要将握持模型的偏移直接用于灯源，否则会重新产生偏心和近景显形范围变化。

无效灯源、关灯、无效资源或缺少注册时，目标保持隐藏。不要通过停用组件 Tick 来代替关灯，否则会保留上一帧材质状态。

## 材质与目标生命周期

默认材质为 `/FlashlightGhostReveal/Materials/M_GhostReveal`，另提供 `MI_GhostReveal`。它是低饱和冷白色 Unlit Masked，clip=0.5，支持普通静态及骨骼网格，不是真实半透明。

默认配色为低饱和冷白色：固定线性色相 `(0.65, 0.82, 0.88)`，亮度随 Coverage 从外圈的 0.35 平滑增加到中心的 1.0。外圈不再单独变成高饱和蓝色；保留基础亮度，使整个 Support 范围仍可见。这里的 35% 是亮度系数，不是透明度。

每个材质槽建立独立 MID；重复绑定相同 Mesh 不重复建立。`Unbind Target` 和 `Unbind All` 恢复绑定前的材质、CastShadow、CustomDepth 设置。组件结束时也会恢复。若游戏要更换 Mesh 或材质槽，先解绑，再修改并重新绑定。注销深度源用 `Unregister Target`；注销后仍绑定的目标隐藏，恢复原外观需要再解绑。

`Strength` 是 OpacityMask 的乘数，不是透明度；修改后调用 `Refresh Style`。低于裁切阈值会使目标隐藏，不能用它实现连续透明渐变。

美术自定义材质时，将 `MF_RevealConeCoverage` 的 **Support** 输出与 `MF_RevealLightVisibility`、Strength 相乘后接入 OpacityMask。Support 在几何外光锥及有效距离内为 1，外部为 0；原有 **Coverage** 输出保留连续衰减，只用于颜色等外观，不能再直接接入 0.5 裁切，否则会重新缩小显形范围。使用主材质同名参数及打包方式，并保持默认有效值为 0。修改渲染路径、WPO/PDO、双面或混合模式需要接入方确认表现。

## 参数与边界

| 参数 | 默认值及约定 |
|---|---|
| 深度分辨率 | 2048；可用 512、1024、2048 |
| Filter Samples | 1；可用 1、4、9 |
| Bias Cm | 0.5 cm；有限且非负 |
| Near Clip Cm | 1 cm；有限且大于 0 |
| 灯角度 | UE Spot 半角；0 ≤ inner < outer < 90° |
| 距离 | UE 世界单位 cm；范围跟随 AttenuationRadius |
| 调度 | 同帧捕获，再发布匹配的深度纹理及参数 |
| History Policy | 默认 PreserveMainView；可选 ResetMainViewWhenDepthInvalidates |

可选历史重置会在有效深度失效时重置整个主视图的时域历史，可能影响其他画面内容，不是局部 TSR 修复。

首版面向单主视图、单手电筒、普通网格、Opaque/Masked 挡光物、Deferred。多个鬼魂可以注册到同一灯源；不提供多灯合成。半透明挡光、鬼魂自遮挡、光源近裁面内的遮挡物、Nanite 目标未作为保证范围。

历史检查发现过 TSR 极少量边缘和关灯残留，本次不宣称任意轨迹逐像素无残留。没有本插件版本的正式性能预算结论；旧仅光锥结果不能代表当前完整效果成本。
