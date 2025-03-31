#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// Nano configuration

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

//normal pins main
#define BOOT_BUTTON_GPIO        GPIO_NUM_0   //boot 打断对话
#define POWER_ADC_PIN           GPIO_NUM_8   //电源电压
#define TOUCH_BUTTON_GPIO       GPIO_NUM_1   //预留的touch 
#define BUILTIN_LED_GPIO        GPIO_NUM_21  //电源指示灯？ 预留的 还没做电路


//normal pins
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_40 //音量加
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_39 //音量减
#define RGB_LED_GPIO            GPIO_NUM_45 //RGB灯


#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_6       // IO6
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_12      // IO12 对应 LRCK
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_14      // IO14 对应 SCLK
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_11      // IO11 对应 DIN
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_13      // IO13 对应 DOUT

//音频编解码 I2C 接口（ES8311）
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_5
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_4
#define AUDIO_CODEC_PA_PIN       GPIO_NUM_2  // 放大器开关

#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR


//LCD pins
#define DISPLAY_SPI_DC_PIN      GPIO_NUM_7    // IO7
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_15   // IO15
#define DISPLAY_SPI_SCLK_PIN    GPIO_NUM_16   // IO16
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_17   // IO17
#define DISPLAY_SPI_RESET_PIN   GPIO_NUM_18   // IO18

#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false

#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_36  //屏幕背光
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false // true: 低电平点亮，false: 高电平点亮

#define DISPLAY_SPI_SCLK_HZ     (40 * 1000 * 1000)

#endif // _BOARD_CONFIG_H_
