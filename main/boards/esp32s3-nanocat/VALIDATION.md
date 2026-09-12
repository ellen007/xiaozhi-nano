# NanoCat migration validation

Date: 2026-09-12. Source baseline: nano-2026 `bbe6e08`, containing official
v2.5.0 `ac6deed` and subsequent upstream fixes through `184a688`.

- Independent ESP-IDF 6.1 / Python 3.11 environment; old IDF 5.3.2 retained.
- Host tests: 81 passed. Five existing tests now restore the current directory
  before TemporaryDirectory cleanup, which is necessary on Windows.
- NanoCat initial complete build passed: application 2,711,520 bytes, assets
  1,262,750 bytes. Final DIO configuration rebuild also passed.
- NanoCat binary: `self.upgrade_firmware` absent. OTA object has no unresolved
  `esp_ota_begin` or `esp_ota_write` references. Both firmware upgrade Kconfig
  options disabled; Hi Miaomiao enabled and Nihao Xiaozhi disabled.
- Existing Nano reference variant complete build passed. Its manual upgrade
  tool and both OTA write references remain present, confirming opt-out is
  scoped correctly. The reference firmware was not flashed to NanoCat.
- Touched C/C++ files pass repository clang-format checks.

## Device validation

All five generated images were written over USB and verified by esptool 5.4.0.
NVS was preserved. Startup reports application 2.5.0, ES8311 initialization
success, saved volume90 and wn9_himiaomiao_tts. The user confirmed normal
chatting. Logs show wake, microphone transcription, multiple replies, completed
speaking-to-listening transitions and eventual return to idle.

One explicit brownout occurred during the first startup. The subsequent boot
completed; the following 120-second passive recording showed no further reset.
This is a remaining power-stability concern, not a software crash diagnosis.
Cold power cycling, BOOT interruption, volume changes and reconnect were not
separately exercised on this new version. Do not treat this as full endurance
validation or proof that earlier intermittent power/I2C faults are cured.

Final application SHA256:
`6ab5826f073b647864a5125c0380bd249991289b1675ad2ba32c00c6e8524900`

Assets SHA256:
`9220ada2c42f50066968ef0d9c085d870a624e991d350abbf3ea7c149adf3063`

The complete 16 MB pre-migration image is stored locally under
`D:/GitHub/nano-device-backup/working-hi-miaomiao-full-20260912.bin`, SHA256
`3f08d9b67aaa70ff61587f7fc4ba4891f8f043a63ba2117bb24e9043201379ab`.
Its application and wake-model slices match the previously tested backups
byte for byte. It contains private NVS and must not be uploaded to Git.
The old working firmware remains the rollback baseline.

Local build/flash evidence is under `D:/GitHub/nano-build-tools/` and
`D:/GitHub/nano-device-backup/`; candidate binaries and their exact manifest
are in `nanocat-v2.5.0-candidate-20260912` under the latter directory.
