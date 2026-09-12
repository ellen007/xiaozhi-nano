# Xiaozhi Nano · 自制语音开发板

本升级分支 `upgrade/nanocat-v2.5.0-no-ota` 新增 **NanoCat V1.0（2025-03-22 接线）**，
基于小智 v2.5.0 及原 nano-2026 修复。NanoCat 背光为 GPIO3，音频 RX13 / TX11，
只有 BOOT 按钮，设备端固件升级全部禁用；具体构建和回退步骤见
[NanoCat 说明](main/boards/esp32s3-nanocat/README.md)。以下 Nano / Nano Pro 的接线警告
针对原有两款板，不代表 NanoCat 需要改线。新版仍须完成实板验证后才能作为稳定版使用。

基于 [小智 ESP32](https://github.com/78/xiaozhi-esp32) 的自制开发板固件适配项目，维护 **Nano** 与 **Nano Pro** 两款 ESP32-S3 开发板。

当前开发分支为 [`nano-2026`](https://github.com/ellen007/xiaozhi-nano/tree/nano-2026)，用于将板级代码迁移到新版小智。历史工程保留在 [`main`](https://github.com/ellen007/xiaozhi-nano/tree/main)，旧版应用功能和自定义资源尚未全部迁移。

> **当前为源码适配阶段，尚未提供经过实板验证的可烧录固件。**
> 原配置中的 GPIO36 背光、Pro 的 GPIO37 SD_D2 与 N16R8 Octal PSRAM 存在冲突，需要核对实际 PCB 并处理接线。当前背光 PWM 默认关闭、SD 未挂载；软件关闭外设不会解除板上的物理负载。

## 当前进度

截至 2026-09-11，适配基于上游提交 [`184a688`](https://github.com/78/xiaozhi-esp32/commit/184a688cd04564c15f035cee09c0f61889fc8e9d)。

| 项目 | 状态 |
| --- | --- |
| Nano / Nano Pro 板级代码与构建注册 | 已加入 |
| Nano / Nano Pro 自动固件升级 | 已关闭，保留联网激活和手动升级 |
| 显示、音频、Wi-Fi、MCP 接口迁移 | 已按该上游版本更新 |
| Pro 六颗 WS2812、音量键 | 已加入驱动，待实板验证 |
| 上游主机测试 | 81 项通过，记录于 2026-09-11 |
| 板型识别、补丁应用、格式检查 | 已通过 |
| ESP-IDF 完整固件编译 | 待完成 |
| 烧录、音频、屏幕、按键、网络及 OTA 实测 | 待完成 |
| 硬件引脚冲突与供电发热排查 | 待完成 |

主机测试检查的是构建脚本等逻辑，不能代替目标固件编译和硬件测试。

## 选择你的开发板

两款板均按 **ESP32-S3 N16R8（16 MB Flash / 8 MB PSRAM）** 配置，使用 ES8311 音频编解码器、GC9A01 240×240 圆屏。

| 配置 | Nano | Nano Pro |
| --- | --- | --- |
| 板目录 | `esp32s3-nano` | `esp32s3-nano-pro` |
| 固件变体 | `esp32s3-nano-2026-n16r8` | `esp32s3-nano-pro-2026-n16r8` |
| ESP32 I2S 接收 RX | GPIO11 | GPIO13 |
| ESP32 I2S 发送 TX | GPIO13 | GPIO11 |
| 音量 + / - | GPIO40 / GPIO39 | GPIO40 / GPIO39 |
| RGB 灯 | 默认关闭，数量待核对 | GPIO45，6 颗 WS2812 |
| GPIO1 预留触摸 | 默认关闭 | 未启用 |
| SD 卡 | 无驱动 | 保留引脚定义，未挂载 |
| 背光 PWM | 默认关闭 | 默认关闭 |

**两款板的 I2S 接线不同，不能混刷。** RX/TX 方向均以 ESP32 为准；屏幕外观相同不代表板型相同。

详细引脚和功能开关见 [Nano 配置](main/boards/esp32s3-nano/config.h)、[Nano Pro 配置](main/boards/esp32s3-nano-pro/config.h)。

## 获取代码

在新目录中克隆本分支，保留原来的工程、旧固件和编译环境，方便比较与回退：

```sh
git clone --branch nano-2026 https://github.com/ellen007/xiaozhi-nano.git xiaozhi-nano-2026
cd xiaozhi-nano-2026
```

如果已经克隆了该分支，在工作区干净时更新：

```sh
git pull --ff-only
```

## 编译

### 准备环境

本分支要求 **ESP-IDF 6.0.1 或更高版本，优先使用 6.1**，不支持 ESP-IDF 5.x。

Windows、macOS 和 Linux 均可使用。Windows 上建议在单独目录中安装所需 SDK，通过已配置的 ESP-IDF 终端执行命令；先核对旧环境版本，不直接覆盖原工程或复制旧的 `build/` 目录。

在已激活的 ESP-IDF 环境中确认版本：

```sh
idf.py --version
python --version
```

以下示例使用该环境中的 `python`；若本机命令名为 `python3`，相应替换即可。

### 选择板型构建

先查看可用板型：

```sh
python scripts/build.py --list-boards
```

**Nano Pro：**

```sh
python scripts/build.py esp32s3-nano-pro --name esp32s3-nano-pro-2026-n16r8 --language zh-CN
```

**Nano：**

```sh
python scripts/build.py esp32s3-nano --name esp32s3-nano-2026-n16r8 --language zh-CN
```

按实际硬件选择一个命令。构建脚本会配置目标芯片、板型和固件变体。语言与唤醒词属于用户构建选项，不固化在板型 JSON 中；其他参数可运行 `python scripts/build.py --help` 查看。

当前基线的 16 MB Flash、Octal PSRAM / 80 MHz 和 `partitions/v2/16m.csv` 已由项目默认配置提供。更换模组或升级上游后，需要重新核对这些设置。

成功构建后，脚本会生成 `build/merged-binary.bin`。本分支目前**尚未验证生成该固件**，上述命令是操作说明，不是成功编译记录。

## 烧录前与首次测试

1. 核对实际 PCB、模组型号、I2S 接线，处理 GPIO36 / GPIO37 冲突及异常发热。
2. 备份原固件和需要保留的数据，核对新旧分区布局。
3. 使用与本板匹配的构建产物进行首次串口烧录，再检查启动日志、Flash / PSRAM 初始化。
4. 逐项验证低音量录放音、屏幕方向、灯串、按键、配网、唤醒与打断。
5. 最后核对服务端按自定义 `type` / `name` 分发固件的行为，再按需验证手动 OTA。

首次从旧工程迁移时，不直接套用其他开发板的预编译固件，也不假定旧 OTA 包与当前分区兼容。具体烧录端口与地址应以本次构建产物和实际连接为准。

## 固件升级策略

Nano 和 Nano Pro 的构建配置均设置 `CONFIG_AUTO_FIRMWARE_UPGRADE=n`。联网时仍请求 OTA 服务以完成激活、获取服务配置和时间，但跳过自动固件安装，包括服务端返回 `force: 1` 的情况。升级检查不会从 GitHub 自动拉取代码。

手动固件升级工具和资源包更新仍保留。此设置需要重新编译并烧录后才对真机生效，已经安装的旧固件不会因仓库修改而改变。其他板型保持上游自动升级的默认行为。

## 已知限制

- **背光与内存引脚：** 原定义 `DISPLAY_BACKLIGHT_PIN=GPIO36` 与 N16R8 Octal PSRAM 冲突。当前不创建 PWM，因此屏幕可能不亮；直接打开该引脚的背光开关会触发编译期检查。应先解决硬件接线，再更新板型或变体。
- **Pro SD 卡：** SD_D2=GPIO37 同样占用内存引脚，当前没有挂载代码。仅禁用驱动不能消除卡槽、上拉等实际负载。
- **电池电量：** Pro 的 GPIO8 分压采样对应 5V 电源轨，不能直接推算电芯剩余电量，因此未启用电池百分比。
- **预留触摸：** GPIO1 的实际触摸方案仍需确认。数字按键驱动不能直接替代原生电容触摸驱动。
- **设备端 AEC：** 未启用，需要经验证的回采链路与声学条件。
- **供电发热：** 固件适配不能修复短路、错料、散热不足或电源切换问题，应单独检查供电与焊接。

## 项目文档

- [Nano 2026 适配与维护说明](docs/nano2026.md)
- [Nano 开发板说明](main/boards/esp32s3-nano/README.md)
- [Nano Pro 开发板说明](main/boards/esp32s3-nano-pro/README.md)
- [小智自定义开发板教程](docs/custom-board_zh.md)
- [音频模块说明](main/audio/README.md)
- [MCP 协议](docs/mcp-protocol.md)
- [上游中文项目介绍](README_zh.md)（通用功能介绍，不代表 Nano 已实现全部能力）

## 后续维护

`nano-2026` 用于当前适配开发，`main` 保留历史版本。同步上游时，在适配分支中审查、合并并重新验证；需要合并回历史分支时另行安排。

硬件改版时应建立独立板型或固件变体，记录接线变化，保持 OTA 身份明确。修改引脚前先核对原理图、PCB 和外设占用。

## 上游与许可证

本项目基于 [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)，感谢上游及相关开源组件的贡献者。

保留原项目的 [MIT 许可证及版权声明](LICENSE)。各第三方组件遵循其各自许可证。
