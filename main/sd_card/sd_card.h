#ifndef DISPLAY_H
#define DISPLAY_H

#include <SD_MMC.h>
#include <SPI.h>
#include <TFT_eSPI.h> // 假设使用 TFT_eSPI 库驱动 LCD

#define SD_CS_PIN 5 // SD 卡的片选引脚
#define DISPLAY_SPI_SCLK_HZ 40000000 // SPI 时钟频率 40 MHz

extern TFT_eSPI tft; // LCD 显示对象

bool initSDCard();
void loadEmotionFrames(const char* emotionDir);
void displayEmotion(const char* emotionName);

#endif // DISPLAY_H