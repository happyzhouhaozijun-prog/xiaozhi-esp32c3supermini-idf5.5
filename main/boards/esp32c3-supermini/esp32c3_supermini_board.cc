#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"

#include "application.h"
#include "button.h"
//#include "led/single_led.h"
#include "led/circular_strip.h"
#include "config.h"



#include <wifi_station.h>


//#include <driver/spi_common.h>

#define TAG "ESP32C3SuperMiniBoard"

/**
 * ESP32-C3 SuperMini 开发板配置
 * 
 * 硬件配置:
 * - MAX98357A I2S功放
 * - INMP441 I2S麦克风
 * - ST7789 SPI LCD (240x240)
 * - 4MB Flash
 * - 单按键交互
 */
class Esp32C3SuperMiniBoard : public WifiBoard {
private:
    Button boot_button_;
 

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }
/*
    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        thing_manager.AddThing(iot::CreateThing("Lamp"));
        thing_manager.AddThing(iot::CreateThing("ChipInfo"));  // ← 添加
        //thing_manager.AddThing(iot::CreateThing("MyFan"));

    }*/

public:
    Esp32C3SuperMiniBoard() : boot_button_(BOOT_BUTTON_GPIO) {

        InitializeButtons();
        //InitializeIot();
    }

    virtual Led* GetLed() override {
        
        
        static CircularStrip led(BUILTIN_LED_GPIO,8);
        return &led;
    }



    virtual AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
    }


};

DECLARE_BOARD(Esp32C3SuperMiniBoard);
