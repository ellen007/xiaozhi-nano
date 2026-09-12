# NanoCat migration validation

Date: 2026-09-12. Source baseline: nano-2026 `bbe6e08`, containing official
v2.5.0 `ac6deed` and subsequent upstream fixes through `184a688`.

- Independent ESP-IDF 6.1 / Python 3.11 environment; old IDF 5.3.2 retained.
- Host tests: 81 passed. Five existing tests now restore the current directory
  before TemporaryDirectory cleanup, which is necessary on Windows.
- NanoCat initial complete build passed: application 2,711,520 bytes, assets
  1,262,750 bytes. Final DIO configuration rebuild is tracked separately.
- NanoCat binary: `self.upgrade_firmware` absent. OTA object has no unresolved
  `esp_ota_begin` or `esp_ota_write` references. Both firmware upgrade Kconfig
  options disabled; Hi Miaomiao enabled and Nihao Xiaozhi disabled.
- Existing Nano reference variant complete build passed. Its manual upgrade
  tool and both OTA write references remain present, confirming opt-out is
  scoped correctly. The reference firmware was not flashed to NanoCat.
- Touched C/C++ files pass repository clang-format checks.

Physical validation of the new firmware is still pending. The old working
firmware remains the rollback baseline; this document does not certify long-term
stability or establish the cause of earlier power/I2C failures.
