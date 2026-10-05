#include "circular_strip.h"
#include "application.h"
#include <esp_log.h>

#define TAG "CircularStrip"

#define DEFAULT_BRIGHTNESS 4
#define HIGH_BRIGHTNESS 16
#define LOW_BRIGHTNESS 1

#define BLINK_INFINITE -1

CircularStrip::CircularStrip(gpio_num_t gpio, uint8_t max_leds) : max_leds_(max_leds) {
    // If the gpio is not connected, you should use NoLed class
    assert(gpio != GPIO_NUM_NC);

    colors_.resize(max_leds_);

    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = gpio;
    strip_config.max_leds = max_leds_;
    strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB;
    strip_config.led_model = LED_MODEL_WS2812;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_));
    led_strip_clear(led_strip_);

    esp_timer_create_args_t strip_timer_args = {
        .callback = [](void *arg) {
            auto strip = static_cast<CircularStrip*>(arg);
            std::lock_guard<std::mutex> lock(strip->mutex_);
            if (strip->strip_callback_ != nullptr) {
                strip->strip_callback_();
            }
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Strip Timer",
        .skip_unhandled_events = false,
    };
    ESP_ERROR_CHECK(esp_timer_create(&strip_timer_args, &strip_timer_));
}

CircularStrip::~CircularStrip() {
    esp_timer_stop(strip_timer_);
    if (led_strip_ != nullptr) {
        led_strip_del(led_strip_);
    }
}

//静态颜色	StaticColor(color)	所有 LED 显示同一种颜色，常亮
void CircularStrip::StaticColor(StripColor color) {
    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(strip_timer_);
    for (int i = 0; i < max_leds_; i++) {
        colors_[i] = color;
        led_strip_set_pixel(led_strip_, i, color.red, color.green, color.blue);
    }
    led_strip_refresh(led_strip_);
}
//闪烁	Blink(color, interval_ms)	所有 LED 同时亮/灭，周期闪烁
void CircularStrip::Blink(StripColor color, int interval_ms) {
    for (int i = 0; i < max_leds_; i++) {
        colors_[i] = color;
    }
    StartStripTask(interval_ms, [this]() {
        static bool on = true;
        if (on) {
            for (int i = 0; i < max_leds_; i++) {
                led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
            }
            led_strip_refresh(led_strip_);
        } else {
            led_strip_clear(led_strip_);
        }
        on = !on;
    });
}
//渐隐	FadeOut(interval_ms)	从当前颜色逐渐变暗直到熄灭
void CircularStrip::FadeOut(int interval_ms) {
    StartStripTask(interval_ms, [this]() {
        bool all_off = true;
        for (int i = 0; i < max_leds_; i++) {
            colors_[i].red /= 2;
            colors_[i].green /= 2;
            colors_[i].blue /= 2;
            if (colors_[i].red != 0 || colors_[i].green != 0 || colors_[i].blue != 0) {
                all_off = false;
            }
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
        if (all_off) {
            led_strip_clear(led_strip_);
            esp_timer_stop(strip_timer_);
        } else {
            led_strip_refresh(led_strip_);
        }
    });
}
//呼吸	Breathe(low, high, interval_ms)	颜色在低亮度和高亮度之间渐变
void CircularStrip::Breathe(StripColor low, StripColor high, int interval_ms) {
    StartStripTask(interval_ms, [this, low, high]() {
        static bool increase = true;
        static StripColor color = low;
        if (increase) {
            if (color.red < high.red) {
                color.red++;
            }
            if (color.green < high.green) {
                color.green++;
            }
            if (color.blue < high.blue) {
                color.blue++;
            }
            if (color.red == high.red && color.green == high.green && color.blue == high.blue) {
                increase = false;
            }
        } else {
            if (color.red > low.red) {
                color.red--;
            }
            if (color.green > low.green) {
                color.green--;
            }
            if (color.blue > low.blue) {
                color.blue--;
            }
            if (color.red == low.red && color.green == low.green && color.blue == low.blue) {
                increase = true;
            }
        }
        for (int i = 0; i < max_leds_; i++) {
            led_strip_set_pixel(led_strip_, i, color.red, color.green, color.blue);
        }
        led_strip_refresh(led_strip_);
    });
}
//滚动/流水	Scroll(low, high, length, interval_ms)	一段高亮光点在 LED 上滚动
void CircularStrip::Scroll(StripColor low, StripColor high, int length, int interval_ms) {
    for (int i = 0; i < max_leds_; i++) {
        colors_[i] = low;
    }
    StartStripTask(interval_ms, [this, low, high, length]() {
        static int offset = 0;
        for (int i = 0; i < max_leds_; i++) {
            colors_[i] = low;
        }
        for (int j = 0; j < length; j++) {
            int i = (offset + j) % max_leds_;
            colors_[i] = high;
        }
        for (int i = 0; i < max_leds_; i++) {
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
        led_strip_refresh(led_strip_);
        offset = (offset + 1) % max_leds_;
    });
}

//length这个是某一刻显示多少点
//interval_ms 这个是每个状态停留多久
//low high都是3个变量，是一个结构体
void CircularStrip::Scroll2(StripColor low, StripColor high, int length, int interval_ms) {
    for (int i = 0; i < max_leds_; i++) {
        colors_[i] = low;
    }
    StartStripTask(interval_ms, [this, low, high, length]() {
        static int offset = 0;
        for (int i = 0; i < max_leds_; i++) {
            colors_[i] = low;
        }
        for (int j = 0; j < length; j++) {
            int i = (offset + j) % max_leds_;
            colors_[i] = high;
        }
        for (int i = 0; i < max_leds_; i++) {
            led_strip_set_pixel(led_strip_, i, colors_[i].red, colors_[i].green, colors_[i].blue);
        }
        led_strip_refresh(led_strip_);
        offset = (offset + 1) % max_leds_;
    });
}




void CircularStrip::StartStripTask(int interval_ms, std::function<void()> cb) {
    if (led_strip_ == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    esp_timer_stop(strip_timer_);
    
    strip_callback_ = cb;
    esp_timer_start_periodic(strip_timer_, interval_ms * 1000);
}


void CircularStrip::OnStateChanged() {
    auto& app = Application::GetInstance();
    auto device_state = app.GetDeviceState();
    switch (device_state) {
        //Starting	kDeviceStateStarting	Scroll2 蓝色滚动 系统启动中
        case kDeviceStateStarting: {
            //StripColor low = { 0, 0, 0 };
            //StripColor high = { LOW_BRIGHTNESS, LOW_BRIGHTNESS, DEFAULT_BRIGHTNESS };
            //Scroll(low, high, 3, 100);
            StripColor low = { 0, 0, 0 };
            StripColor high = { 1, 2, 2 };
            Scroll2(low, high, 5, 100);

            break;
        }
        //WifiConfiguring	kDeviceStateWifiConfiguring	Blink 蓝色闪烁 	配网模式
        case kDeviceStateWifiConfiguring: {
            StripColor color = { LOW_BRIGHTNESS, LOW_BRIGHTNESS, DEFAULT_BRIGHTNESS };
            Blink(color, 500);
            break;
        }
        //Idle	kDeviceStateIdle	Breathe 蓝色呼吸 待命
        case kDeviceStateIdle:{//待机状态。
            //FadeOut(50);
            StripColor low = { 0,0,0};
            StripColor high = { 0, 0, DEFAULT_BRIGHTNESS };
            Breathe(low, high, 500);  // 蓝色呼吸灯
            break;
        }
        //Connecting	kDeviceStateConnecting	StaticColor 蓝色常亮	连接服务器
        case kDeviceStateConnecting: {
            StripColor color = { LOW_BRIGHTNESS, LOW_BRIGHTNESS, DEFAULT_BRIGHTNESS };
            StaticColor(color);
            break;
        }
        //Listening	kDeviceStateListening	StaticColor 红色常亮	聆听中
        case kDeviceStateListening: {
            StripColor color = { DEFAULT_BRIGHTNESS, LOW_BRIGHTNESS, LOW_BRIGHTNESS };
            StaticColor(color);
            break;
        }
        //Speaking	kDeviceStateSpeaking	StaticColor 绿色常亮	说话中
        case kDeviceStateSpeaking: {
            StripColor color = { LOW_BRIGHTNESS, DEFAULT_BRIGHTNESS, LOW_BRIGHTNESS };
            StaticColor(color);
            break;
        }
        //Upgrading	kDeviceStateUpgrading	Blink 绿色快闪	OTA 升级
        case kDeviceStateUpgrading: {
            StripColor color = { LOW_BRIGHTNESS, DEFAULT_BRIGHTNESS, LOW_BRIGHTNESS };
            Blink(color, 100);
            break;
        }
        //Activating	kDeviceStateActivating	Blink 绿色慢闪	设备激活
        case kDeviceStateActivating: {
            StripColor color = { LOW_BRIGHTNESS, DEFAULT_BRIGHTNESS, LOW_BRIGHTNESS };
            Blink(color, 500);
            break;
        }
        default:
            ESP_LOGW(TAG, "Unknown led strip event: %d", device_state);
            return;
    }
}
