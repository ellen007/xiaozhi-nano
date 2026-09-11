#include "application.h"
#include "assets/lang_config.h"
#include "backlight.h"
#include "button.h"
#include "codecs/es8311_audio_codec.h"
#include "config.h"
#include "display/lcd_display.h"
#include "led/circular_strip.h"
#include "wifi_board.h"

#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <esp_lcd_gc9a01.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_log.h>
#include <algorithm>
#include <string>

namespace {
constexpr const char* TAG = "Esp32S3Nano";

class NanoRoundDisplay : public SpiLcdDisplay {
public:
    using SpiLcdDisplay::SpiLcdDisplay;

    void SetupUI() override {
        // Current upstream creates widgets here, after board construction.
        SpiLcdDisplay::SetupUI();
        DisplayLockGuard lock(this);
        // A 160px bar 36px inside the 240px circle leaves room for corners.
        if (top_bar_) {
            lv_obj_set_width(top_bar_, NANO_ROUND_BAR_WIDTH);
            lv_obj_align(top_bar_, LV_ALIGN_TOP_MID, 0, NANO_ROUND_BAR_INSET);
        }
        if (status_bar_) {
            lv_obj_set_width(status_bar_, NANO_ROUND_BAR_WIDTH);
            lv_obj_align(status_bar_, LV_ALIGN_TOP_MID, 0, NANO_ROUND_BAR_INSET + 24);
        }
        if (status_label_) {
            lv_obj_set_width(status_label_, NANO_ROUND_BAR_WIDTH - 8);
        }
        if (notification_label_) {
            lv_obj_set_width(notification_label_, NANO_ROUND_BAR_WIDTH - 8);
        }
        if (bottom_bar_) {
            lv_obj_set_width(bottom_bar_, NANO_ROUND_BAR_WIDTH);
            lv_obj_align(bottom_bar_, LV_ALIGN_BOTTOM_MID, 0, -NANO_ROUND_BAR_INSET);
        }
        if (chat_message_label_) {
            lv_obj_set_width(chat_message_label_, NANO_ROUND_BAR_WIDTH - 8);
        }
    }
};

class Esp32S3Nano : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_ = nullptr;
    Display* display_ = nullptr;
    Button boot_button_{BOOT_BUTTON_GPIO};
#if NANO_ENABLE_VOLUME_BUTTONS
    Button volume_up_button_{VOLUME_UP_BUTTON_GPIO};
    Button volume_down_button_{VOLUME_DOWN_BUTTON_GPIO};
#endif
#if NANO_ENABLE_TOUCH_BUTTON
    Button touch_button_{TOUCH_BUTTON_GPIO, NANO_TOUCH_ACTIVE_HIGH};
#endif

    void InitializeCodecI2c() {
        i2c_master_bus_config_t config = {};
        config.i2c_port = I2C_NUM_0;
        config.sda_io_num = AUDIO_CODEC_I2C_SDA_PIN;
        config.scl_io_num = AUDIO_CODEC_I2C_SCL_PIN;
        config.clk_source = I2C_CLK_SRC_DEFAULT;
        config.glitch_ignore_cnt = 7;
        config.flags.enable_internal_pullup = true;
        ESP_ERROR_CHECK(i2c_new_master_bus(&config, &codec_i2c_bus_));
    }

    void InitializeDisplay() {
        spi_bus_config_t bus = {};
        bus.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        bus.miso_io_num = GPIO_NUM_NC;
        bus.sclk_io_num = DISPLAY_SPI_SCK_PIN;
        bus.quadwp_io_num = GPIO_NUM_NC;
        bus.quadhd_io_num = GPIO_NUM_NC;
        bus.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO));

        esp_lcd_panel_io_handle_t io = nullptr;
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_SPI_DC_PIN;
        io_config.spi_mode = 0;
        io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &io));

        esp_lcd_panel_handle_t panel = nullptr;
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RESET_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io, &panel_config, &panel));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR));
        // SpiLcdDisplay owns rotation/mirror and display-on initialization.
        display_ = new NanoRoundDisplay(io, panel, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X,
                                        DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
                                        DISPLAY_SWAP_XY);
    }

    void SetVolume(int volume) {
        const int bounded = std::clamp(volume, 0, 100);
        GetAudioCodec()->SetOutputVolume(bounded);
        display_->ShowNotification(std::string(Lang::Strings::VOLUME) + std::to_string(bounded));
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            Application::GetInstance().Schedule([this]() {
                auto& app = Application::GetInstance();
                if (app.GetDeviceState() == kDeviceStateStarting) {
                    EnterWifiConfigMode();
                    return;
                }
                app.ToggleChatState();
            });
        });
        // Explicit recovery path, also available after normal startup.
        boot_button_.OnLongPress([this]() { EnterWifiConfigMode(); });
#if NANO_ENABLE_VOLUME_BUTTONS
        volume_up_button_.OnClick([this]() {
            Application::GetInstance().Schedule(
                [this]() { SetVolume(GetAudioCodec()->output_volume() + NANO_VOLUME_STEP); });
        });
        volume_down_button_.OnClick([this]() {
            Application::GetInstance().Schedule(
                [this]() { SetVolume(GetAudioCodec()->output_volume() - NANO_VOLUME_STEP); });
        });
        volume_up_button_.OnLongPress(
            [this]() { Application::GetInstance().Schedule([this]() { SetVolume(100); }); });
        volume_down_button_.OnLongPress(
            [this]() { Application::GetInstance().Schedule([this]() { SetVolume(0); }); });
#endif
#if NANO_ENABLE_TOUCH_BUTTON
        // These Application methods dispatch event bits and are thread-safe.
        touch_button_.OnPressDown([]() { Application::GetInstance().StartListening(); });
        touch_button_.OnPressUp([]() { Application::GetInstance().StopListening(); });
#endif
    }

public:
    Esp32S3Nano() {
        InitializeCodecI2c();
        InitializeDisplay();
        InitializeButtons();
        // Speaker and brightness MCP tools are supplied by AddCommonTools().
        if (auto* backlight = GetBacklight()) {
            backlight->RestoreBrightness();
        } else {
            ESP_LOGW(TAG, "Backlight PWM disabled: legacy GPIO36 conflicts with N16R8 PSRAM");
        }
    }

    Display* GetDisplay() override { return display_; }

    Backlight* GetBacklight() override {
#if NANO_ENABLE_BACKLIGHT
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
#else
        return nullptr;
#endif
    }

    Led* GetLed() override {
#if NANO_ENABLE_RGB_LED
        static CircularStrip led(RGB_LED_GPIO, NANO_RGB_LED_COUNT);
#else
        static NoLed led;
#endif
        return &led;
    }

    AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec codec(
            codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT,
            AUDIO_I2S_GPIO_DIN, AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &codec;
    }
};
}  // namespace

DECLARE_BOARD(Esp32S3Nano);
