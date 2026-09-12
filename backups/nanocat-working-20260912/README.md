# NanoCat 已通过实际测试的备份（2026-09-12）

分支：`backup/nanocat-working-hi-miaomiao-20260912`。项目版本 1.5.1，ESP-IDF 5.3.2。
实际构建板型为 `movecall-moji-esp32s3`，BOARD_NAME 为 `movecall-moji-esp32s3-no-auto-ota`；并非 Nano Pro，也不是仓库中新增的 NanoCat 板型。

## 保留的状态

- 保留本次诊断修改以及此前用户的引脚注释、表情与字体依赖修改。
- 关闭自动固件升级，保留激活及服务器配置检查。此备份不声称已经封堵所有远程升级入口。
- ES8311：SDA5、SCL4、MCLK6、BCLK14、WS12、DOUT11、DIN13、功放9；采样率24000。
- GC9A01 240x240：DC7、CS15、SCLK16、MOSI17、RESET18、背光3；BOOT0。
- 保留时钟优先、I2C恢复及失败重试诊断。背光10%；音量仅首次设置80，随后保留用户设置；最近日志音量90。
- 唤醒词为“Hi 喵喵”，资源为 wn9_himiaomiao_tts。

用户反馈正常。串口已验证 ES8311 初始化成功、“Hi,喵喵”触发、收到回复、speaking 后回到 listening 并继续对话。
这代表本次实际测试通过，不是长期稳定性保证；之前出现过欠压复位和间歇性 I2C 故障，根因尚未完全确定。

## 固件与配置

`application.bin` 是最后实际刷入且校验通过的应用，4240496 字节。
`hi-miaomiao-model.bin` 是随后单独刷入且校验通过的唤醒模型，291039 字节。
应用在更换唤醒模型前已编译，因此应用与更新后的模型是分别保存的；不要使用旧构建目录中的模型覆盖本文件。
`sdkconfig.snapshot` 为更换唤醒词后的完整构建配置，`sdkconfig.board.defaults` 为板型覆盖配置，`dependencies.lock.snapshot` 为依赖锁定快照。
其余三个 bin 来自该构建目录，作为启动及分区配套资料保存，不是本次重新从设备读取的全量镜像。
每个文件大小及 SHA256 见 `sha256.json`。

## 回退

先保存未来工作区中的修改，再切换本备份分支即可恢复源码：

```powershell
git switch backup/nanocat-working-hi-miaomiao-20260912
```

切换 Git 分支不会改变设备。恢复设备还需 USB 刷写。
只有设备仍使用以下旧版分区布局时，才可仅恢复应用与模型：
模型 0x10000/0xF0000；ota_0 0x100000/0x600000；ota_1 0x700000/0x600000。
使用 esptool 4.8.1，在本目录执行（COM7需按实际串口替换）：

```powershell
python -m esptool --chip esp32s3 --port COM7 --baud 115200 --before default_reset --after hard_reset write_flash 0x100000 application.bin 0x10000 hi-miaomiao-model.bin
```

上述命令假定仍启动 ota_0。若未来迁移新版并更换分区布局、启动槽位或 NVS 格式，需先核对并恢复配套启动/分区状态，不能直接套用此命令。
此 Git 备份不包含设备 NVS、Wi-Fi 密码、绑定数据或当前完整16MB Flash镜像；迁移前应另做完整设备备份。

## 重新构建

先安装 ESP-IDF 5.3.2 并加载 export.ps1，在仓库根目录：

```powershell
Copy-Item backups/nanocat-working-20260912/dependencies.lock.snapshot dependencies.lock
idf.py -B build/restore-nanocat -D SDKCONFIG=build/restore-nanocat/sdkconfig -D "SDKCONFIG_DEFAULTS=backups/nanocat-working-20260912/sdkconfig.snapshot;backups/nanocat-working-20260912/sdkconfig.board.defaults" -D IDF_TARGET=esp32s3 -D BOARD_NAME=movecall-moji-esp32s3-no-auto-ota build
```

依赖源码需由组件管理器取得。重新编译可能因时间戳、路径及环境不同产生不同哈希；直接使用保存的二进制才是恢复本次原始产物。
