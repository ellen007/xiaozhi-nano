#include "application.h"
#include "button.h"
#include "codecs/es8311_audio_codec.h"
#include "config.h"
#include "display/lcd_display.h"
#include "settings.h"
#include "wifi_board.h"

#include <esp_rom_sys.h>
#include <esp_system.h>

#include <driver/i2c_master.h>
#include <esp_efuse_table.h>
#include <esp_log.h>

#include <esp_lcd_gc9a01.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"

#define TAG "NanoCat"

class NanoCat : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    Display* display_;

    // Run once before the controller owns the pins. Open-drain only: never
    // drive a stuck SDA/SCL high. Recovery is bounded to nine clock pulses.
    void ReleaseCodecBus() {
        gpio_config_t pins = {};
        pins.pin_bit_mask = (1ULL << AUDIO_CODEC_I2C_SDA_PIN) | (1ULL << AUDIO_CODEC_I2C_SCL_PIN);
        pins.mode = GPIO_MODE_INPUT_OUTPUT_OD;
        pins.pull_up_en = GPIO_PULLUP_ENABLE;
        ESP_ERROR_CHECK(gpio_set_level(AUDIO_CODEC_I2C_SDA_PIN, 1));
        ESP_ERROR_CHECK(gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 1));
        ESP_ERROR_CHECK(gpio_config(&pins));
        esp_rom_delay_us(10);
        int pulses = 0;
        while (!gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN) && pulses < 9 &&
               gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN)) {
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 0);
            esp_rom_delay_us(10);
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 1);
            esp_rom_delay_us(10);
            ++pulses;
        }
        if (gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN)) {
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 0);
            gpio_set_level(AUDIO_CODEC_I2C_SDA_PIN, 0);
            esp_rom_delay_us(10);
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 1);
            esp_rom_delay_us(10);
            gpio_set_level(AUDIO_CODEC_I2C_SDA_PIN, 1);
            esp_rom_delay_us(10);
        }
        ESP_LOGI(TAG, "Codec bus release: pulses=%d SDA=%d SCL=%d", pulses,
                 gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN), gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN));
        ESP_ERROR_CHECK(gpio_reset_pin(AUDIO_CODEC_I2C_SDA_PIN));
        ESP_ERROR_CHECK(gpio_reset_pin(AUDIO_CODEC_I2C_SCL_PIN));
    }

    void InitializeCodecI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags =
                {
                    .enable_internal_pullup = 1,
                },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    // SPI初始化
    void InitializeSpi() {
        ESP_LOGI(TAG, "Initialize SPI bus");
        spi_bus_config_t buscfg =
            GC9A01_PANEL_BUS_SPI_CONFIG(DISPLAY_SPI_SCLK_PIN, DISPLAY_SPI_MOSI_PIN,
                                        DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    // GC9A01初始化
    void InitializeGc9a01Display() {
        ESP_LOGI(TAG, "Init GC9A01 display");

        ESP_LOGI(TAG, "Install panel IO");
        esp_lcd_panel_io_handle_t io_handle = NULL;
        esp_lcd_panel_io_spi_config_t io_config =
            GC9A01_PANEL_IO_SPI_CONFIG(DISPLAY_SPI_CS_PIN, DISPLAY_SPI_DC_PIN, NULL, NULL);
        io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &io_handle));

        ESP_LOGI(TAG, "Install GC9A01 panel driver");
        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RESET_PIN;  // Set to -1 if not use
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
        panel_config.bits_per_pixel = 16;  // Implemented by LCD command `3Ah` (16/18)

        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

        display_ = new SpiLcdDisplay(io_handle, panel_handle, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                     DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X,
                                     DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            app.Schedule([this, &app]() {
                if (app.GetDeviceState() == kDeviceStateStarting) {
                    EnterWifiConfigMode();
                    return;
                }
                app.ToggleChatState();
            });
        });
    }

public:
    NanoCat() : boot_button_(BOOT_BUTTON_GPIO) {
        // Keep the amplifier muted until the codec explicitly enables playback.
        ESP_ERROR_CHECK(gpio_set_level(AUDIO_CODEC_PA_PIN, 0));
        ESP_ERROR_CHECK(gpio_set_direction(AUDIO_CODEC_PA_PIN, GPIO_MODE_OUTPUT));
        ESP_LOGI(TAG, "Boot reset reason: %d; USB firmware updates only",
                 static_cast<int>(esp_reset_reason()));
        ReleaseCodecBus();
        InitializeCodecI2c();
        InitializeSpi();
        InitializeGc9a01Display();
        InitializeButtons();
        // Match the tested legacy startup load without overwriting saved settings.
        GetBacklight()->SetBrightness(10, false);
        Settings audio_settings("audio", false);
        ESP_LOGI(TAG, "Saved output volume: %d", audio_settings.GetInt("output_volume", 70));
    }

    // Board::GetLed() returns NoLed. GPIO36 belongs to octal PSRAM on N16R8;
    // GPIO21 was a legacy Moji placeholder, not the NanoCat schematic LED.
    virtual Display* GetDisplay() override { return display_; }

    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(
            codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT,
            AUDIO_I2S_GPIO_DIN, AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }
};

DECLARE_BOARD(NanoCat);
