#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// Nano configuration

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

//normal pins main
#define BOOT_BUTTON_GPIO         GPIO_NUM_0   //boot for touch
#define BUILTIN_LED_GPIO         GPIO_NUM_21  //


//normal pins
#define VOLUME_UP_BUTTON_GPIO    GPIO_NUM_40  //
#define VOLUME_DOWN_BUTTON_GPIO  GPIO_NUM_39  //-
#define RGB_LED_GPIO             GPIO_NUM_45  //RGB


#define AUDIO_I2S_GPIO_MCLK      GPIO_NUM_6   // IO6 MCLK
#define AUDIO_I2S_GPIO_WS        GPIO_NUM_12  // IO12 LRCK
#define AUDIO_I2S_GPIO_BCLK      GPIO_NUM_14  // IO14 SCLK
#define AUDIO_I2S_GPIO_DIN       GPIO_NUM_13  // IO11 DIN
#define AUDIO_I2S_GPIO_DOUT      GPIO_NUM_11  // IO13 DOUT

//音频编解码 I2C 接口（ES8311）
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_5   // IO05 SDA
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_4   // IO04 SCL
#define AUDIO_CODEC_PA_PIN       GPIO_NUM_2   // IO02 CTRL  

#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR  // 


// SD Card pins
#define SD_CMD_PIN              GPIO_NUM_10  // IO10  CMD
#define SD_CLK_PIN              GPIO_NUM_9   // IO09  CLK
#define SD_D0_PIN               GPIO_NUM_46  // IO46  D0
#define SD_D1_PIN               GPIO_NUM_3   // IO03  D1
#define SD_D2_PIN               GPIO_NUM_37  // IO37  D2
#define SD_D3_PIN               GPIO_NUM_38  // IO38  D3

//LCD pins
#define DISPLAY_SPI_DC_PIN      GPIO_NUM_7    // IO7  DC
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_15   // IO15 CS
#define DISPLAY_SPI_SCK_PIN     GPIO_NUM_16   // IO16 SCK 
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_17   // IO17 MOSI
#define DISPLAY_SPI_RESET_PIN   GPIO_NUM_18   // IO18 RESET

#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false

#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_BACKLIGHT_PIN   GPIO_NUM_36  //screen backlight
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false // true: LED lights up on low level，false: LED lights up on high level


#define DISPLAY_SPI_SCLK_HZ     (40 * 1000 * 1000)

#endif // _BOARD_CONFIG_H_
