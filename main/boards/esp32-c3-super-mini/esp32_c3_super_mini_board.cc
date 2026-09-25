#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/oled_display.h"
#include "application.h"
#include "button.h"
#include "mcp_server.h"
#include "settings.h"
#include "config.h"
#include "press_to_talk_mcp_tool.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_sh1106.h>

#define TAG "EspC3SuperMiniBoard"

class EspC3SuperMiniBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t display_i2c_bus_ = nullptr;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    Display* display_ = nullptr;
    Button boot_button_;
    PressToTalkMcpTool* press_to_talk_tool_ = nullptr;

    void InitializeDisplayI2c() {
        i2c_master_bus_config_t bus_config = {};
        bus_config.i2c_port = DISPLAY_I2C_PORT;
        bus_config.sda_io_num = DISPLAY_SDA_PIN;
        bus_config.scl_io_num = DISPLAY_SCL_PIN;
        bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_config.glitch_ignore_cnt = 7;
        bus_config.intr_priority = 0;
        bus_config.trans_queue_depth = 0;
        bus_config.flags.enable_internal_pullup = 1;
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus_));
    }

    void InitializeSh1106Display() {
        // IDF 5.5 and 6.0 declare these fields in a different order. Assign
        // them individually so C++ designated-initializer ordering is irrelevant.
        esp_lcd_panel_io_i2c_config_t io_config = {};
        io_config.dev_addr = DISPLAY_I2C_ADDR;
        io_config.scl_speed_hz = 400 * 1000;
        io_config.control_phase_bytes = 1;
        io_config.dc_bit_offset = 6;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        io_config.on_color_trans_done = nullptr;
        io_config.user_ctx = nullptr;
        io_config.flags.dc_low_on_data = 0;
        io_config.flags.disable_control_phase = 0;

        if (esp_lcd_new_panel_io_i2c(display_i2c_bus_, &io_config, &panel_io_) != ESP_OK) {
            ESP_LOGE(TAG, "SH1106 not found on I2C 0x%02X (SDA=%d SCL=%d), running without display",
                     DISPLAY_I2C_ADDR, (int)DISPLAY_SDA_PIN, (int)DISPLAY_SCL_PIN);
            display_ = new NoDisplay();
            return;
        }

        ESP_LOGI(TAG, "Install SH1106 driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.bits_per_pixel = 1;
        // The SH1106 driver is fixed 128x64 and needs no vendor_config; it adds
        // the module's 2-column RAM offset (132-column RAM) inside draw_bitmap().
        ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(panel_io_, &panel_config, &panel_));
        ESP_LOGI(TAG, "SH1106 driver installed");

        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        if (esp_lcd_panel_init(panel_) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize display");
            display_ = new NoDisplay();
            return;
        }

        ESP_LOGI(TAG, "Turning display on");
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

        display_ = new OledDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                   DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            if (!press_to_talk_tool_ || !press_to_talk_tool_->IsPressToTalkEnabled()) {
                app.ToggleChatState();
            }
        });
        boot_button_.OnPressDown([this]() {
            if (press_to_talk_tool_ && press_to_talk_tool_->IsPressToTalkEnabled()) {
                Application::GetInstance().StartListening();
            }
        });
        boot_button_.OnPressUp([this]() {
            if (press_to_talk_tool_ && press_to_talk_tool_->IsPressToTalkEnabled()) {
                Application::GetInstance().StopListening();
            }
        });
    }

    void InitializeTools() {
        press_to_talk_tool_ = new PressToTalkMcpTool();
        press_to_talk_tool_->Initialize();
    }

public:
    EspC3SuperMiniBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeDisplayI2c();
        InitializeSh1106Display();
        InitializeButtons();
        InitializeTools();
    }

    virtual AudioCodec* GetAudioCodec() override {
        // INMP441 + MAX98357A share BCLK/WS -> single duplex I2S port.
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
};

DECLARE_BOARD(EspC3SuperMiniBoard);
