# esp32s3-nano

适配上游 `184a688cd04564c15f035cee09c0f61889fc8e9d`（2026-09-11），ESP-IDF 6.0.1 以上，优先 6.1。不是给 2025 年旧分支直接替换用的文件。

**保留 GitHub 原接线的接口迁移版；GPIO36 背光与已确认的 N16R8 冲突，默认关闭 PWM，屏幕可能不亮。须核对实物接线。**

## 硬件与功能

- ESP32-S3 N16R8、ES8311、GC9A01 240×240，SPI3 / 40 MHz、BGR、反色和镜像沿用原设置。
- BOOT 单击切换对话；启动阶段单击进入配网并立即返回；长按进入配网。
- 音量键单击 ±10，长按分别为最大音量 / 静音，通过主任务处理。
- 所有可配置接线和功能开关集中在 `config.h`。
- 背光：`NANO_ENABLE_BACKLIGHT=0`。启用时通过 `GetBacklight()` 提供 PWM 与 MCP 亮度控制；关闭时返回空能力，不宣称可调亮度。
- RGB 默认关闭。确认对应 GPIO 上确实连接 WS2812 后才启用 `NANO_ENABLE_RGB_LED`。普通指示灯不能用 `SingleLed` 驱动。
- 轻触默认关闭；`Button` 支持数字电平输入，不是原生电容触摸电极驱动。启用前核对接线和有效电平。
- 电量显示与设备端 AEC 未启用，前者缺少可靠采样电路，后者缺少经验证的回采链路。

## 构建

本分支已包含板目录、Kconfig 与 CMake 注册。不要把旧目录中的其他 `.cc` 文件叠加复制进来。

```bash
python3 scripts/build.py esp32s3-nano --name esp32s3-nano-2026-n16r8 --language zh-CN
```

N16R8 的 16 MB Flash、Octal PSRAM / 80 MHz 和 v2/16m 分区已由本次固定上游 defaults 提供，因此 `sdkconfig_append` 留空。语言和唤醒词通过构建参数选择。

板型 `type` 与固件 `name` 已区分；首次迁移应核对分区和备份数据后通过串口烧录。服务端是否严格按自定义板型/变体路由 OTA 仍需验证，仅改名称不等于保证隔离。

本分支的构建说明与验证范围见 `docs/nano2026.md`。
