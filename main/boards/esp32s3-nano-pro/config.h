#pragma once

#include <driver/gpio.h>
#include "sdkconfig.h"

// N16R8 confirmed by owner. Keep PCB pins; do not guess replacements.
#define AUDIO_INPUT_SAMPLE_RATE 24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000
#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_6
#define AUDIO_I2S_GPIO_WS GPIO_NUM_12
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_14
// Direction is from ESP32: DIN receives codec ADC, DOUT sends to codec DAC.
#define AUDIO_I2S_GPIO_DIN GPIO_NUM_13
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_11
#define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_5
#define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_4
#define AUDIO_CODEC_PA_PIN GPIO_NUM_2
#define AUDIO_CODEC_ES8311_ADDR ES8311_CODEC_DEFAULT_ADDR

#define BOOT_BUTTON_GPIO GPIO_NUM_0
#define VOLUME_UP_BUTTON_GPIO GPIO_NUM_40
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_39
#define NANO_ENABLE_VOLUME_BUTTONS 1
#define NANO_VOLUME_STEP 10

// GPIO21 is a reserved expansion signal, not the WS2812 data input.
#define BUILTIN_LED_GPIO GPIO_NUM_21
#define RGB_LED_GPIO GPIO_NUM_45
#define NANO_ENABLE_RGB_LED 1
// Owner schematic: LED1..LED6 are chained on RGB_C / GPIO45.
#define NANO_RGB_LED_COUNT 6

#define DISPLAY_SPI_DC_PIN GPIO_NUM_7
#define DISPLAY_SPI_CS_PIN GPIO_NUM_15
#define DISPLAY_SPI_SCK_PIN GPIO_NUM_16
#define DISPLAY_SPI_MOSI_PIN GPIO_NUM_17
#define DISPLAY_SPI_RESET_PIN GPIO_NUM_18
#define DISPLAY_SPI_SCLK_HZ (40 * 1000 * 1000)
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false
#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0
#define DISPLAY_INVERT_COLOR true
#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_36
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false
// GPIO36 is occupied by Octal PSRAM on N16R8. Verify PCB before enabling.
#define NANO_ENABLE_BACKLIGHT 0
#define NANO_ROUND_BAR_WIDTH 160
#define NANO_ROUND_BAR_INSET 36

// GPIO8 divider measures VCC_5V / 2, not the lithium battery cell.
#define POWER_ADC_PIN GPIO_NUM_8

// Reference only: SD is NOT initialized. D2 conflicts with N16R8 PSRAM.
#define SD_CMD_PIN GPIO_NUM_10
#define SD_CLK_PIN GPIO_NUM_9
#define SD_D0_PIN GPIO_NUM_46
#define SD_D1_PIN GPIO_NUM_3
#define SD_D2_PIN GPIO_NUM_37
#define SD_D3_PIN GPIO_NUM_38

#if defined(CONFIG_SPIRAM_MODE_OCT) || defined(CONFIG_ESPTOOLPY_OCT_FLASH)
#if NANO_ENABLE_BACKLIGHT
static_assert(DISPLAY_BACKLIGHT_PIN < GPIO_NUM_33 || DISPLAY_BACKLIGHT_PIN > GPIO_NUM_37,
              "Nano: GPIO33..37 belong to Octal memory; verify/rework backlight wiring.");
#endif
#endif
static_assert(AUDIO_I2S_GPIO_DIN != AUDIO_I2S_GPIO_DOUT, "I2S RX and TX must use different pins.");
static_assert(NANO_VOLUME_STEP > 0 && NANO_VOLUME_STEP <= 100, "Invalid volume step.");

static_assert(NANO_RGB_LED_COUNT > 0 && NANO_RGB_LED_COUNT <= 255, "Invalid RGB LED count.");
