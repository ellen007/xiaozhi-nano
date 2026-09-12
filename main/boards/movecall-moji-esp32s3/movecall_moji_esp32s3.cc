#include "wifi_board.h"
#include "audio_codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "settings.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_rom_sys.h>
#include <esp_efuse_table.h>
#include <driver/i2c_master.h>
#include <driver/i2s_std.h>

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_gc9a01.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"

#define TAG "MovecallMojiESP32S3"

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);


class CustomLcdDisplay : public SpiLcdDisplay {
public:
    CustomLcdDisplay(esp_lcd_panel_io_handle_t io_handle, 
                    esp_lcd_panel_handle_t panel_handle,
                    int width,
                    int height,
                    int offset_x,
                    int offset_y,
                    bool mirror_x,
                    bool mirror_y,
                    bool swap_xy) 
        : SpiLcdDisplay(io_handle, panel_handle, width, height, offset_x, offset_y, mirror_x, mirror_y, swap_xy,
                    {
                        .text_font = &font_puhui_20_4,
                        .icon_font = &font_awesome_20_4,
                        .emoji_font = font_emoji_64_init(),
                    }) {

        DisplayLockGuard lock(this);
        // 由于屏幕是圆的，所以状态栏需要增加左右内边距
        lv_obj_set_style_pad_left(status_bar_, LV_HOR_RES * 0.33, 0);
        lv_obj_set_style_pad_right(status_bar_, LV_HOR_RES * 0.33, 0);
    }
};

class MovecallMojiESP32S3 : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    Display* display_;

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
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    void CheckReleasedCodecPins() {
        // No codec/device handles exist yet; remove the controller before changing pins.
        esp_err_t removed = i2c_del_master_bus(codec_i2c_bus_);
        if (removed != ESP_OK) {
            ESP_LOGE(TAG, "PIN_RELEASE: bus removal failed: %s", esp_err_to_name(removed));
            return;
        }
        codec_i2c_bus_ = nullptr;
        ESP_ERROR_CHECK(gpio_reset_pin(AUDIO_CODEC_I2C_SDA_PIN));
        ESP_ERROR_CHECK(gpio_reset_pin(AUDIO_CODEC_I2C_SCL_PIN));
        gpio_config_t inputs = {};
        inputs.pin_bit_mask = (1ULL << AUDIO_CODEC_I2C_SDA_PIN) | (1ULL << AUDIO_CODEC_I2C_SCL_PIN);
        inputs.mode = GPIO_MODE_INPUT;
        inputs.pull_up_en = GPIO_PULLUP_DISABLE;
        inputs.pull_down_en = GPIO_PULLDOWN_DISABLE;
        inputs.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&inputs));
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_LOGW(TAG, "PIN_RELEASE: controller removed, input only, external pullups: SDA=%d SCL=%d",
                 gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN), gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN));
        inputs.pull_up_en = GPIO_PULLUP_ENABLE;
        ESP_ERROR_CHECK(gpio_config(&inputs));
        unsigned sda_high = 0, scl_high = 0;
        for (int sample = 0; sample < 20; ++sample) {
            vTaskDelay(pdMS_TO_TICKS(10));
            sda_high += gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN);
            scl_high += gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN);
        }
        ESP_LOGW(TAG, "PIN_RELEASE: input only, internal pullups: SDA high=%u/20 SCL high=%u/20",
                 sda_high, scl_high);
        // Independent bus-clear attempt, using open drain only: level 1 releases a line.
        ESP_ERROR_CHECK(gpio_set_level(AUDIO_CODEC_I2C_SDA_PIN, 1));
        ESP_ERROR_CHECK(gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 1));
        inputs.mode = GPIO_MODE_INPUT_OUTPUT_OD;
        ESP_ERROR_CHECK(gpio_config(&inputs));
        esp_rom_delay_us(10);
        unsigned pulses = 0;
        bool clock_released = gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN);
        while (clock_released && !gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN) && pulses < 9) {
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 0);
            esp_rom_delay_us(10);
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 1);
            esp_rom_delay_us(10);
            clock_released = gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN);
            ++pulses;
        }
        if (clock_released) {
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 0);
            gpio_set_level(AUDIO_CODEC_I2C_SDA_PIN, 0);
            esp_rom_delay_us(10);
            gpio_set_level(AUDIO_CODEC_I2C_SCL_PIN, 1);
            esp_rom_delay_us(10);
            gpio_set_level(AUDIO_CODEC_I2C_SDA_PIN, 1);
        }
        // Return to input even if the clock could not be released.
        inputs.mode = GPIO_MODE_INPUT;
        ESP_ERROR_CHECK(gpio_config(&inputs));
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_LOGW(TAG, "MANUAL_CLEAR: pulses=%u clock_released=%d final SDA=%d SCL=%d",
                 pulses, clock_released, gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN),
                 gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN));
        InitializeCodecI2c();
    }

    // SPI初始化
    void InitializeSpi() {
        ESP_LOGI(TAG, "Initialize SPI bus");
        spi_bus_config_t buscfg = GC9A01_PANEL_BUS_SPI_CONFIG(DISPLAY_SPI_SCLK_PIN, DISPLAY_SPI_MOSI_PIN, 
                                    DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    // GC9A01初始化
    void InitializeGc9a01Display() {
        ESP_LOGI(TAG, "Init GC9A01 display");

        ESP_LOGI(TAG, "Install panel IO");
        esp_lcd_panel_io_handle_t io_handle = NULL;
        esp_lcd_panel_io_spi_config_t io_config = GC9A01_PANEL_IO_SPI_CONFIG(DISPLAY_SPI_CS_PIN, DISPLAY_SPI_DC_PIN, NULL, NULL);
        io_config.pclk_hz = DISPLAY_SPI_SCLK_HZ;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &io_handle));
    
        ESP_LOGI(TAG, "Install GC9A01 panel driver");
        esp_lcd_panel_handle_t panel_handle = NULL;
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_SPI_RESET_PIN;    // Set to -1 if not use
        panel_config.rgb_endian = LCD_RGB_ENDIAN_BGR;           //LCD_RGB_ENDIAN_RGB;
        panel_config.bits_per_pixel = 16;                       // Implemented by LCD command `3Ah` (16/18)

        ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true)); 

        display_ = new SpiLcdDisplay(io_handle, panel_handle,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
                                    {
                                        .text_font = &font_puhui_20_4,
                                        .icon_font = &font_awesome_20_4,
                                        .emoji_font = font_emoji_64_init(),
                                    });
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker")); 
        thing_manager.AddThing(iot::CreateThing("Screen"));   
    }

public:
    MovecallMojiESP32S3() : boot_button_(BOOT_BUTTON_GPIO) {  
        // Keep the amplifier off until codec initialization, then test low-volume playback.
        ESP_ERROR_CHECK(gpio_set_level(AUDIO_CODEC_PA_PIN, 0));
        ESP_ERROR_CHECK(gpio_set_direction(AUDIO_CODEC_PA_PIN, GPIO_MODE_OUTPUT));
        Settings audio_settings("audio", true);
        // Apply the requested volume once; subsequent boots preserve user adjustments.
        if (audio_settings.GetInt("diag_vol80", 0) == 0) {
            audio_settings.SetInt("output_volume", 80);
            audio_settings.SetInt("diag_vol80", 1);
        }
        ESP_LOGW(TAG, "AUDIO_DIAG: saved volume %ld; backlight 10%%; reset reason %d",
                 static_cast<long>(audio_settings.GetInt("output_volume", 80)),
                 static_cast<int>(esp_reset_reason()));
        InitializeCodecI2c();
        InitializeSpi();
        InitializeGc9a01Display();
        InitializeButtons();
        InitializeIot();
        GetBacklight()->SetBrightness(10, false);
    }

    virtual Led* GetLed() override {
        static SingleLed led_strip(BUILTIN_LED_GPIO);
        return &led_strip;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
    
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static const bool probed = [this]() {
            display_->SetStatus("音频通信检查");
            // Diagnostic clock source only; release it before the normal codec owns I2S0.
            i2s_chan_handle_t diagnostic_tx = nullptr;
            i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
            chan.auto_clear = true;
            ESP_ERROR_CHECK(i2s_new_channel(&chan, &diagnostic_tx, nullptr));
            i2s_std_config_t clocks = {};
            clocks.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_OUTPUT_SAMPLE_RATE);
            clocks.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
            clocks.gpio_cfg.mclk = AUDIO_I2S_GPIO_MCLK;
            clocks.gpio_cfg.bclk = AUDIO_I2S_GPIO_BCLK;
            clocks.gpio_cfg.ws = AUDIO_I2S_GPIO_WS;
            clocks.gpio_cfg.dout = AUDIO_I2S_GPIO_DOUT;
            clocks.gpio_cfg.din = I2S_GPIO_UNUSED;
            ESP_ERROR_CHECK(i2s_channel_init_std_mode(diagnostic_tx, &clocks));
            ESP_ERROR_CHECK(i2s_channel_enable(diagnostic_tx));
            ESP_LOGW(TAG, "CLOCK_FIRST: I2S running at %d Hz, amplifier still off", AUDIO_OUTPUT_SAMPLE_RATE);
            vTaskDelay(pdMS_TO_TICKS(2000));
            unsigned attempt = 0;
            while (true) {
                if (attempt % 3 == 0) {
                    CheckReleasedCodecPins();
                }
                esp_err_t reset = i2c_master_bus_reset(codec_i2c_bus_);
                vTaskDelay(pdMS_TO_TICKS(100));
                int sda = gpio_get_level(AUDIO_CODEC_I2C_SDA_PIN);
                int scl = gpio_get_level(AUDIO_CODEC_I2C_SCL_PIN);
                esp_err_t result = i2c_master_probe(codec_i2c_bus_, AUDIO_CODEC_ES8311_ADDR >> 1, 100);
                ESP_LOGW(TAG, "BUS_RECOVERY: attempt=%u reset=%s SDA=%d SCL=%d probe=%s boot_reason=%d",
                         ++attempt, esp_err_to_name(reset), sda, scl, esp_err_to_name(result),
                         static_cast<int>(esp_reset_reason()));
                if (result == ESP_OK) {
                    display_->SetStatus("音频芯片已应答");
                    break;
                }
                display_->SetStatus("音频通信故障");
                display_->SetChatMessage("system", "正在重试，请保持供电");
                // Keep the failed bus observable instead of triggering an assertion reboot.
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
            ESP_ERROR_CHECK(i2s_channel_disable(diagnostic_tx));
            ESP_ERROR_CHECK(i2s_del_channel(diagnostic_tx));
            return true;
        }();
        (void)probed;
        static Es8311AudioCodec audio_codec(codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }
};

DECLARE_BOARD(MovecallMojiESP32S3);
