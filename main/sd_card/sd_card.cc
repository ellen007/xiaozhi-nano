#include "sd_card.h"
#include <SD_MMC.h>
#include <vector>

// 初始化 SD 卡
bool SDCard::init() {
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD 卡初始化失败！");
        return false;
    }
    Serial.println("SD 卡初始化成功！");
    return true;
}

// 读取文件内容
std::vector<uint8_t> SDCard::readFile(const char* path) {
    std::vector<uint8_t> data;
    File file = SD_MMC.open(path, FILE_READ);
    if (!file) {
        Serial.printf("无法打开文件: %s\n", path);
        return data;
    }

    // 读取文件内容
    size_t fileSize = file.size();
    data.resize(fileSize);
    file.read(data.data(), fileSize);
    file.close();

    Serial.printf("成功读取文件: %s, 大小: %d 字节\n", path, fileSize);
    return data;
}