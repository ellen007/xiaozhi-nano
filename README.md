# Xiaozhi Nano / NanoCat

基于 [小智 ESP32](https://github.com/78/xiaozhi-esp32) 的自制语音设备项目。本分支以 **小智 v2.5.0** 和原 `nano-2026` 修复为基础，完成了 **NanoCat V1.0** 的接线适配、USB 升级限制和自定义猫脸表情迁移。

**截至 2026-09-12：NanoCat 已编译、刷入并完成实际唤醒、连续聊天和猫脸显示测试。** 当前唤醒词为 **“Hi 喵喵”**。验证范围和已知问题见 [实测记录](main/boards/esp32s3-nanocat/VALIDATION.md)，不将短时测试视为长期稳定性保证。

## 分支与版本

| 分支 | 用途 |
| --- | --- |
| [`upgrade/nanocat-v2.5.0-no-ota`](https://github.com/ellen007/xiaozhi-nano/tree/upgrade/nanocat-v2.5.0-no-ota) | 当前 NanoCat v2.5.0 适配与猫脸表情 |
| [`backup/nanocat-working-hi-miaomiao-20260912`](https://github.com/ellen007/xiaozhi-nano/tree/backup/nanocat-working-hi-miaomiao-20260912) | 已测试的旧版 1.5.1 源码、固件、配置和唤醒模型备份，提交 `a838986` |
| [`nano-2026`](https://github.com/ellen007/xiaozhi-nano/tree/nano-2026) | 原 Nano / Nano Pro 新版适配基线 |
| [`main`](https://github.com/ellen007/xiaozhi-nano/tree/main) | 历史工程，包含原来的 128 像素表情调用方案 |

硬件和固件变体不同，不要仅凭屏幕外观选择板型。此分支的设备测试结果仅针对下面的 NanoCat 接线。

## NanoCat 硬件

使用 **ESP32-S3-WROOM-1-N16R8**（16 MB Flash / 8 MB PSRAM）、**ES8311** 音频芯片和 **GC9A01 240×240 圆屏**，对应创建于 2025-03-07、更新于 2025-03-22 的 NanoCat V1.0 原理图。

| 功能 | GPIO |
| --- | --- |
| ES8311 SDA / SCL | 5 / 4 |
| 音频 MCLK / BCLK / WS | 6 / 14 / 12 |
| ESP32 音频发送 TX / 接收 RX | 11 / 13 |
| 功放使能 | 9，高电平有效 |
| 屏幕 MOSI / SCLK / CS / DC / RESET | 17 / 16 / 15 / 7 / 18 |
| 屏幕背光 | 3，高电平有效 |
| BOOT 按钮 | 0，低电平有效 |

音频输入、输出均为 24 kHz；Flash 使用 DIO，PSRAM 使用 Octal 模式。触摸 GPIO8 暂未启用；不驱动原理图中的 GPIO36 LED，避免占用 N16R8 内存引脚。

板型标识：`esp32s3-nanocat`；固件变体：`esp32s3-nanocat-usb-only`。
详细配置见 [NanoCat 说明](main/boards/esp32s3-nanocat/README.md) 和 [引脚定义](main/boards/esp32s3-nanocat/config.h)。

## 自定义猫脸表情

已接入原 UI 素材中的 **21 张 128×128 透明 PNG**，覆盖待机、开心、大笑、生气、思考、困倦等状态。原始图片像素保持不变，按服务端情绪名称映射；未知名称回退到待机表情。

- **黑色背景：** 保留原来的白色线条和彩色细节。
- **白色背景：** 自动渲染为黑色单色线条，保留透明度和轮廓。
- 根据主题自动切换，无需两份固件；重启保留已保存的主题。

素材已纳入 Git：[表情目录](main/boards/esp32s3-nanocat/emoji/)、[文件映射与 SHA256](main/boards/esp32s3-nanocat/emoji/manifest.json)。本机原 UI 目录下另有 `NanoCat-新版表情备份-20260912`，包含图片、适配代码和 `黑白主题预览.html` 离线预览。

## 构建

要求 **ESP-IDF 6.0.1 及以上**，本次使用 **ESP-IDF 6.1 / Python 3.11**。旧版回退工程使用 IDF 5.3.2，应保留独立环境。

```sh
git clone --branch upgrade/nanocat-v2.5.0-no-ota https://github.com/ellen007/xiaozhi-nano.git xiaozhi-nanocat
cd xiaozhi-nanocat
```

加载 ESP-IDF 环境后核对版本并构建：

```sh
idf.py --version
python scripts/build.py esp32s3-nanocat --name esp32s3-nanocat-usb-only --language zh-CN --wake-word wn9_himiaomiao_tts
```

语言和唤醒词通过构建参数选择，不固定在板型 JSON 中。`Hi 喵喵` 对应 `wn9_himiaomiao_tts`。

表情版应用为 **2,798,976 字节**，21 张 PNG 已嵌入应用；资源包约 **1.3 MB**，包含唤醒模型等资源。完整构建和 81 项主机测试已通过。构建产物来自所选板型，不能拿 Nano / Nano Pro 固件刷入 NanoCat。

## 升级与回退

**NanoCat 仅通过 USB 安装固件。** 此板型不能开启 `FIRMWARE_UPGRADE`：自动安装、服务端强制安装和远程手动升级工具均已禁用，底层固件写入代码也不参与编译。

联网激活、获取服务器配置及资源包更新仍保留。修改 Git 代码不会自动更新设备。

首次由旧版迁移到 v2 分区时：

1. 完整读取并校验当前设备的 16 MB Flash，备份源码、构建配置及固件。
2. 使用本板构建生成的启动、分区、OTA 元数据、应用和资源文件，按生成的地址刷写。
3. 保留 NVS 配网与绑定数据；不要用带填充的合并镜像覆盖 NVS 区域。
4. 检查启动、唤醒、回复播放及屏幕显示，再进行日常使用测试。

旧版应用位于 `0x100000`，新版应用位于 `0x20000`；布局已经改变。**切换 Git 分支只恢复代码，设备回退还需恢复旧分区及配套固件，或完整旧镜像。** 详见 [回退说明](main/boards/esp32s3-nanocat/README.md#migration-and-rollback)。

本次迁移前已保存完整设备镜像，并确认其中的应用与唤醒模型逐字节匹配旧版测试备份。完整镜像包含 Wi-Fi 和绑定信息，只保存在本机，不提交到 Git。

## 验证与限制

| 项目 | 结果 |
| --- | --- |
| NanoCat 完整构建、81 项主机测试、格式检查 | 通过 |
| 原 Nano 参考板型回归构建 | 通过，保留原有手动升级能力 |
| USB 刷写、固件哈希与分区容量检查 | 通过 |
| Hi 喵喵唤醒、麦克风转写、多轮回复 | 实测通过 |
| 自定义猫脸及黑色主题 | 用户确认显示正常 |
| 白色主题 | 自动着色已实现并编译，尚无单独明确的实板确认 |
| 冷启动反复测试、BOOT 打断、重连、长期运行 | 尚未完成专项验证 |

v2.5.0 首次迁移启动曾记录一次明确欠压复位，随后恢复聊天；表情版刷入后约 100 秒观察未见欠压、崩溃或断言。保留启动阶段功放静音、10% 背光以及有限次数的 I2C 总线恢复，音量沿用用户保存值。此前间歇性供电与 I2C 故障的根因尚未完全确定。

## Nano / Nano Pro

仓库仍保留这两款板的适配代码，但其接线不同于 NanoCat：原 Nano 的音频 RX11 / TX13；Nano Pro 为 RX13 / TX11，另有音量键、灯串等外设。原 GPIO36 背光、Pro GPIO37 SD 接线与 N16R8 内存存在冲突，相关功能仍受限制，不能套用 NanoCat 的实测结论。

- [Nano 开发板说明](main/boards/esp32s3-nano/README.md)
- [Nano Pro 开发板说明](main/boards/esp32s3-nano-pro/README.md)
- [原 Nano 2026 适配记录](docs/nano2026.md)

## 文档与许可证

- [NanoCat 实测记录与固件校验值](main/boards/esp32s3-nanocat/VALIDATION.md)
- [自定义开发板教程](docs/custom-board.md)
- [音频模块说明](main/audio/README.md)
- [MCP 协议](docs/mcp-protocol.md)
- [上游中文介绍](README_zh.md)

保留原项目的 [MIT 许可证及版权声明](LICENSE)。第三方组件和素材遵循各自许可证。
