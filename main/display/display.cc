#include <esp_log.h>
#include <esp_err.h>
#include <string>
#include <cstdlib>

#include "display.h"
#include "board.h"
#include "application.h"
#include "audio_codec.h"
#include "settings.h"

#define TAG "Display"

Display::Display() {
    Settings settings("display");
    brightness_ = settings.GetInt("brightness", 100);

    esp_timer_create_args_t notification_timer_args = {
        .callback = [](void *arg) {
            Display *display = static_cast<Display*>(arg);
            DisplayLockGuard lock(display);
            // 无 display，不做任何事
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Notification Timer",
        .skip_unhandled_events = false,
    };
    ESP_ERROR_CHECK(esp_timer_create(&notification_timer_args, &notification_timer_));

    esp_timer_create_args_t update_display_timer_args = {
        .callback = [](void *arg) {
            Display *display = static_cast<Display*>(arg);
            display->Update();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Update Display Timer",
        .skip_unhandled_events = true,
    };
    ESP_ERROR_CHECK(esp_timer_create(&update_display_timer_args, &update_timer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(update_timer_, 1000000));
}

Display::~Display() {
    if (notification_timer_ != nullptr) {
        esp_timer_stop(notification_timer_);
        esp_timer_delete(notification_timer_);
    }
    if (update_timer_ != nullptr) {
        esp_timer_stop(update_timer_);
        esp_timer_delete(update_timer_);
    }
    // 无 display，不做任何事
}

void Display::SetStatus(const std::string &status) {
    // 无 display，空实现
}

void Display::ShowNotification(const std::string &notification, int duration_ms) {
    // 无 display，空实现
}

void Display::Update() {
    // 无 display，空实现
}

void Display::SetEmotion(const std::string &emotion) {
    // 无 display，空实现
}

void Display::SetIcon(const char* icon) {
    // 无 display，空实现
}

void Display::SetChatMessage(const std::string &role, const std::string &content) {
    // 无 display，空实现
}

void Display::SetBacklight(uint8_t brightness) {
    Settings settings("display", true);
    settings.SetInt("brightness", brightness);
    brightness_ = brightness;
}