# NanoCat V1.0

适用于用户确认的 NanoCat 原理图（创建 2025-03-07，更新 2025-03-22），
ESP32-S3-WROOM-1-N16R8。基于本仓库 1.5.1 旧版程序，使用 ESP-IDF 5.3.2。
板型独立于 Nano/Nano Pro；保留现有表情资源和显示逻辑。

| 功能 | ESP32 GPIO |
| --- | --- |
| ES8311 I2C SDA / SCL | 5 / 4 |
| I2S MCLK / BCLK / LRCK | 6 / 14 / 12 |
| ESP32 接收（ES8311 ASDOUT，图纸 DOUT） | 13 |
| ESP32 发送（ES8311 DSDIN，图纸 DIN） | 11 |
| 功放 CTRL | 9 |
| LCD DC / CS / SCK / MOSI / RESET | 7 / 15 / 16 / 17 / 18 |
| LCD 背光 | 3，高电平点亮 |
| BOOT | 0，低电平有效 |
| TC233A OUT（MCK&BUTTON） | 8，低电平有效、内部上拉 |
| 图纸 LED | 36，仅记录，不驱动 |

BOOT 单击切换对话；启动且未连网时单击 BOOT 进入配网。
触摸片单击切换对话/打断；忽略启动初期事件以避开触摸芯片稳定期。
图纸 TC233A 的 LHO 接 VDD、OHO 接地，输出是开漏、低电平有效的同步信号。
GPIO8 不作为电池 ADC；本板不初始化音量键、RGB 灯或 SD 卡。

## 硬件限制

- GPIO36 由 N16R8 Octal PSRAM 占用，不能初始化为 LED 输出，也不能按 WS2812 驱动。
  本板返回 `NoLed`；软件不驱动不代表已消除实物 LED 对内存信号的电气负载。
- LCD 使用旧固件的 GC9A01、240×240、BGR、反色及镜像配置。
  原理图仅标接口，未标面板控制器；屏幕方向/颜色仍需实机验证。
- 默认关闭自动 OTA，保留服务器激活、时间与连接配置获取。
  如需刷入其他版本，使用串口手动烧录经过核对的固件。

参考：[乐鑫内存引脚](https://documentation.espressif.com/esp-hardware-design-guidelines/en/latest/esp32s3/index.html)、
[富满 TC233A 数据手册](https://xonstorage.blob.core.windows.net/pdf/fuman_tc233a_apr22_xonlink.pdf)。

## 编译

激活 ESP-IDF 5.3.2 后，在项目根目录运行（PowerShell）：

```powershell
idf.py -B build-nanocat -D SDKCONFIG=build-nanocat/sdkconfig -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.esp32s3;main/boards/esp32s3-nanocat/sdkconfig.defaults" -D IDF_TARGET=esp32s3 -D BOARD_NAME=esp32s3-nanocat-v1-no-auto-ota build
```

全新 `build-nanocat` 配置默认选择本板和关闭自动 OTA。已有该目录时应检查
`build-nanocat/sdkconfig`，因为 defaults 不会覆盖已有设置。
也可使用仓库原有发布入口 `python scripts/release.py esp32s3-nanocat`，
但该入口会重建常规 `build` 目录及其配置。

输出位于 `build-nanocat`，与原来的 `build` 分开。
编译通过仅代表软件构建验证；触摸、音频、屏幕、联网及禁用 OTA 的实机验证仍待执行。
