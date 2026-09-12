#include "nanocat_display.h"
#include "display/lvgl_display/lvgl_theme.h"

#include <cstring>

namespace {
extern const uint8_t neutral_start[] asm("_binary_nanocat_neutral_png_start");
extern const uint8_t neutral_end[] asm("_binary_nanocat_neutral_png_end");
extern const uint8_t happy_start[] asm("_binary_nanocat_happy_png_start");
extern const uint8_t happy_end[] asm("_binary_nanocat_happy_png_end");
extern const uint8_t laughing_start[] asm("_binary_nanocat_laughing_png_start");
extern const uint8_t laughing_end[] asm("_binary_nanocat_laughing_png_end");
extern const uint8_t funny_start[] asm("_binary_nanocat_funny_png_start");
extern const uint8_t funny_end[] asm("_binary_nanocat_funny_png_end");
extern const uint8_t sad_start[] asm("_binary_nanocat_sad_png_start");
extern const uint8_t sad_end[] asm("_binary_nanocat_sad_png_end");
extern const uint8_t angry_start[] asm("_binary_nanocat_angry_png_start");
extern const uint8_t angry_end[] asm("_binary_nanocat_angry_png_end");
extern const uint8_t crying_start[] asm("_binary_nanocat_crying_png_start");
extern const uint8_t crying_end[] asm("_binary_nanocat_crying_png_end");
extern const uint8_t loving_start[] asm("_binary_nanocat_loving_png_start");
extern const uint8_t loving_end[] asm("_binary_nanocat_loving_png_end");
extern const uint8_t embarrassed_start[] asm("_binary_nanocat_embarrassed_png_start");
extern const uint8_t embarrassed_end[] asm("_binary_nanocat_embarrassed_png_end");
extern const uint8_t surprised_start[] asm("_binary_nanocat_surprised_png_start");
extern const uint8_t surprised_end[] asm("_binary_nanocat_surprised_png_end");
extern const uint8_t shocked_start[] asm("_binary_nanocat_shocked_png_start");
extern const uint8_t shocked_end[] asm("_binary_nanocat_shocked_png_end");
extern const uint8_t thinking_start[] asm("_binary_nanocat_thinking_png_start");
extern const uint8_t thinking_end[] asm("_binary_nanocat_thinking_png_end");
extern const uint8_t winking_start[] asm("_binary_nanocat_winking_png_start");
extern const uint8_t winking_end[] asm("_binary_nanocat_winking_png_end");
extern const uint8_t cool_start[] asm("_binary_nanocat_cool_png_start");
extern const uint8_t cool_end[] asm("_binary_nanocat_cool_png_end");
extern const uint8_t relaxed_start[] asm("_binary_nanocat_relaxed_png_start");
extern const uint8_t relaxed_end[] asm("_binary_nanocat_relaxed_png_end");
extern const uint8_t delicious_start[] asm("_binary_nanocat_delicious_png_start");
extern const uint8_t delicious_end[] asm("_binary_nanocat_delicious_png_end");
extern const uint8_t kissy_start[] asm("_binary_nanocat_kissy_png_start");
extern const uint8_t kissy_end[] asm("_binary_nanocat_kissy_png_end");
extern const uint8_t confident_start[] asm("_binary_nanocat_confident_png_start");
extern const uint8_t confident_end[] asm("_binary_nanocat_confident_png_end");
extern const uint8_t sleepy_start[] asm("_binary_nanocat_sleepy_png_start");
extern const uint8_t sleepy_end[] asm("_binary_nanocat_sleepy_png_end");
extern const uint8_t silly_start[] asm("_binary_nanocat_silly_png_start");
extern const uint8_t silly_end[] asm("_binary_nanocat_silly_png_end");
extern const uint8_t confused_start[] asm("_binary_nanocat_confused_png_start");
extern const uint8_t confused_end[] asm("_binary_nanocat_confused_png_end");
struct Expression {
    const char* name;
    LvglRawImage image;
};

const LvglRawImage& FindExpression(const char* name) {
    // Descriptors live for the display lifetime; PNG data stays in flash.
    static const Expression expressions[] = {
        {"neutral", LvglRawImage(const_cast<uint8_t*>(neutral_start), neutral_end - neutral_start)},
        {"happy", LvglRawImage(const_cast<uint8_t*>(happy_start), happy_end - happy_start)},
        {"laughing",
         LvglRawImage(const_cast<uint8_t*>(laughing_start), laughing_end - laughing_start)},
        {"funny", LvglRawImage(const_cast<uint8_t*>(funny_start), funny_end - funny_start)},
        {"sad", LvglRawImage(const_cast<uint8_t*>(sad_start), sad_end - sad_start)},
        {"angry", LvglRawImage(const_cast<uint8_t*>(angry_start), angry_end - angry_start)},
        {"crying", LvglRawImage(const_cast<uint8_t*>(crying_start), crying_end - crying_start)},
        {"loving", LvglRawImage(const_cast<uint8_t*>(loving_start), loving_end - loving_start)},
        {"embarrassed", LvglRawImage(const_cast<uint8_t*>(embarrassed_start),
                                     embarrassed_end - embarrassed_start)},
        {"surprised",
         LvglRawImage(const_cast<uint8_t*>(surprised_start), surprised_end - surprised_start)},
        {"shocked", LvglRawImage(const_cast<uint8_t*>(shocked_start), shocked_end - shocked_start)},
        {"thinking",
         LvglRawImage(const_cast<uint8_t*>(thinking_start), thinking_end - thinking_start)},
        {"winking", LvglRawImage(const_cast<uint8_t*>(winking_start), winking_end - winking_start)},
        {"cool", LvglRawImage(const_cast<uint8_t*>(cool_start), cool_end - cool_start)},
        {"relaxed", LvglRawImage(const_cast<uint8_t*>(relaxed_start), relaxed_end - relaxed_start)},
        {"delicious",
         LvglRawImage(const_cast<uint8_t*>(delicious_start), delicious_end - delicious_start)},
        {"kissy", LvglRawImage(const_cast<uint8_t*>(kissy_start), kissy_end - kissy_start)},
        {"confident",
         LvglRawImage(const_cast<uint8_t*>(confident_start), confident_end - confident_start)},
        {"sleepy", LvglRawImage(const_cast<uint8_t*>(sleepy_start), sleepy_end - sleepy_start)},
        {"silly", LvglRawImage(const_cast<uint8_t*>(silly_start), silly_end - silly_start)},
        {"confused",
         LvglRawImage(const_cast<uint8_t*>(confused_start), confused_end - confused_start)},
    };
    for (const auto& expression : expressions) {
        if (name != nullptr && strcmp(name, expression.name) == 0) {
            return expression.image;
        }
    }
    return expressions[0].image;  // neutral fallback
}
}  // namespace

void NanoCatDisplay::ApplyExpressionColor() {
    if (emoji_image_ == nullptr || current_theme_ == nullptr) {
        return;
    }
    const auto* theme = static_cast<LvglTheme*>(current_theme_);
    const bool light = lv_color_brightness(theme->background_color()) > 128;
    lv_obj_set_style_image_recolor(emoji_image_, lv_color_black(), 0);
    lv_obj_set_style_image_recolor_opa(emoji_image_, light ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void NanoCatDisplay::SetupUI() {
    SpiLcdDisplay::SetupUI();
    SetEmotion("neutral");
}

void NanoCatDisplay::SetEmotion(const char* emotion) {
    DisplayLockGuard lock(this);
    if (emoji_image_ == nullptr) {
        return;
    }
    if (gif_controller_) {
        gif_controller_->Stop();
        gif_controller_.reset();
    }
    lv_image_set_src(emoji_image_, FindExpression(emotion).image_dsc());
    ApplyExpressionColor();
    if (emoji_label_ != nullptr) {
        lv_obj_add_flag(emoji_label_, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_remove_flag(emoji_image_, LV_OBJ_FLAG_HIDDEN);
}

void NanoCatDisplay::SetTheme(Theme* theme) {
    SpiLcdDisplay::SetTheme(theme);
    DisplayLockGuard lock(this);
    ApplyExpressionColor();
    ESP_LOGI("NanoCatDisplay", "Cat expression theme: %s", theme->name().c_str());
}
