#include "wifi_board.h"
#include "audio_codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"
#include "led/led.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_gc9a01.h>

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);

class Esp32s3NanoCat : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_ = nullptr;
    Button boot_button_{BOOT_BUTTON_GPIO};
    Button touch_button_{TOUCH_BUTTON_GPIO, TOUCH_BUTTON_ACTIVE_HIGH};
    Display* display_ = nullptr;
    // TC233A needs about 0.5 s to stabilize after power-on.
    int64_t touch_ready_at_ = esp_timer_get_time() + 1000000;

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
        spi_bus_config_t bus_config = GC9A01_PANEL_BUS_SPI_CONFIG(
            DISPLAY_SPI_SCK_PIN, DISPLAY_SPI_MOSI_PIN,
            DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO));

        esp_lcd_panel_io_handle_t io_handle = nullptr;
        esp_lcd_panel_io_spi_config_t io_config = GC9A01_PANEL_IO_SPI_CONFIG(
            DISPLAY_SPI_CS_PIN, DISPLAY_SPI_DC_PIN, nullptr, nullptr);
        io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &io_handle));

        esp_lcd_panel_handle_t panel_handle = nullptr;
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RESET_PIN;
        panel_config.rgb_endian = LCD_RGB_ENDIAN_BGR;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
        display_ = new SpiLcdDisplay(io_handle, panel_handle,
            DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
            {.text_font = &font_puhui_20_4,
             .icon_font = &font_awesome_20_4,
             .emoji_font = font_emoji_64_init()});
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            Application::GetInstance().Schedule([this]() {
                auto& app = Application::GetInstance();
                if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                    ResetWifiConfiguration();
                    return;
                }
                app.ToggleChatState();
            });
        });
        touch_button_.OnClick([this]() {
            if (esp_timer_get_time() < touch_ready_at_) {
                return;
            }
            Application::GetInstance().Schedule([]() {
                Application::GetInstance().ToggleChatState();
            });
        });
    }

public:
    Esp32s3NanoCat() {
        InitializeCodecI2c();
        InitializeDisplay();
        InitializeButtons();
        auto& things = iot::ThingManager::GetInstance();
        things.AddThing(iot::CreateThing("Speaker"));
        things.AddThing(iot::CreateThing("Screen"));
        ESP_LOGI("NanoCat", "V1.0: backlight=3, PA=9, touch=8; GPIO36 LED disabled for Octal PSRAM");
    }

    Led* GetLed() override {
        static NoLed led;
        return &led;
    }

    Display* GetDisplay() override { return display_; }

    Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec codec(codec_i2c_bus_, I2C_NUM_0,
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS,
            AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &codec;
    }
};

DECLARE_BOARD(Esp32s3NanoCat);
