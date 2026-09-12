# NanoCat V1.0 (March 2025)

Independent board support for the enclosed NanoCat built with an
ESP32-S3-WROOM-1-N16R8, ES8311 and a round GC9A01 240x240 display. This is
the schematic created 2025-03-07 and updated 2025-03-22, not Nano or Nano Pro.

The reported type is `esp32s3-nanocat`; the firmware variant is
`esp32s3-nanocat-usb-only`. Do not change this identity to match a stock board.

## Wiring

| Function | GPIO |
| --- | --- |
| ES8311 I2C SDA / SCL | 5 / 4 |
| I2S MCLK / BCLK / WS | 6 / 14 / 12 |
| ESP32 audio output / input | 11 / 13 |
| Amplifier enable (active high) | 9 |
| Display MOSI / clock / CS / DC / reset | 17 / 16 / 15 / 7 / 18 |
| Display backlight (active high) | 3 |
| BOOT button (active low) | 0 |

ES8311 uses I2C address 0x18 (0x30 in the codec API), 24 kHz input/output.
The screen uses BGR, inversion, X mirroring and a 40 MHz SPI clock.
Touch GPIO8 is reserved and unused. No LED output is enabled: GPIO36 on this
N16R8 module is used by octal PSRAM, and GPIO21 is not the schematic LED.

## Build

Use ESP-IDF 6.1 (minimum 6.0.1), then run from the repository root:

```sh
python scripts/build.py esp32s3-nanocat --name esp32s3-nanocat-usb-only --language zh-CN --wake-word wn9_himiaomiao_tts
```

This selects “Hi 喵喵”. Language and wake-word choices are build parameters,
not fixed board configuration. Keep DIO flash mode as in the tested legacy
firmware. Project defaults supply 16 MB flash, octal PSRAM and v2/16m partitions.

## Custom cat expressions

The 21 user-provided transparent 128x128 PNGs in `emoji/` are embedded only
for NanoCat. `emoji/manifest.json` records original filenames and checksums.
`NanoCatDisplay` maps server emotion names to these images with a neutral
fallback. The image descriptors are persistent; changing an emotion does not
allocate another image buffer in the application.

Dark backgrounds preserve the original white artwork and colored accents.
Light backgrounds use LVGL's opaque black recoloring while retaining the PNG
alpha, producing a monochrome version with the same silhouette. The current
theme is loaded from NVS and both startup and later theme changes apply the
matching treatment. No second firmware or duplicate raster set is required.

## Device updates and recovery

`FIRMWARE_UPGRADE` cannot be enabled for this board. The firmware download/write
implementation and remote `self.upgrade_firmware` tool are excluded; the
application entry point also rejects installation before stopping audio.
Automatic and server-forced firmware installation are therefore disabled.
Activation, server settings, asset updates and USB flashing remain available.

Startup mutes GPIO9 before codec initialization and uses 10% backlight without
overwriting persisted settings. Audio volume uses the existing
`audio/output_volume` NVS value. There is no per-boot forced volume write.
The board attempts at most nine open-drain bus-clear pulses before taking
ownership of I2C. The upstream ES8311 codec starts I2S clocks before software
reset. The old diagnostic firmware's indefinite startup retry loop is not
carried forward. A stuck bus can still prevent initialization; this migration
does not establish that previous intermittent hardware/power faults are cured.

## Migration and rollback

The last tested 1.5.1 source and binaries are in branch
`backup/nanocat-working-hi-miaomiao-20260912`, commit `a838986`.
The new branch includes upstream v2.5.0 plus the existing nano-2026 fixes.

Before the first flash, read and verify a complete 16 MB image of the currently
working device. Keep it locally: it includes Wi-Fi and binding information.
Never publish that full image in Git.

The old model at 0x10000 and application at 0x100000 do not match the new layout:
v2 uses ota_0 at 0x20000, ota_1 at 0x410000 and an 8 MB asset partition at
0x800000. Flash all generated bootloader, partition, OTA metadata, application
and assets files using their generated offsets. Preserve NVS at 0x9000 unless
an actual compatibility failure requires a separately reviewed migration.
Do not flash a padded merged image across existing NVS.

A rollback after changing partitions requires the old full image or the entire
old boot/partition/app/model set, not just changing a Git branch or writing the
old application at the new offset. Verify saved image hashes before restoring.

Build success and host tests are not physical validation. Before calling this
version working, verify cold boot, screen, microphone, “Hi 喵喵”, complete reply
playback, volume control, BOOT interruption, reconnect and absence of resets.
