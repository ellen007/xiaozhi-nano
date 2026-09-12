#pragma once

#include "display/lcd_display.h"

class NanoCatDisplay : public SpiLcdDisplay {
public:
    using SpiLcdDisplay::SpiLcdDisplay;
    void SetupUI() override;
    void SetEmotion(const char* emotion) override;
    void SetTheme(Theme* theme) override;

private:
    // Caller holds the LVGL display lock.
    void ApplyExpressionColor();
};
