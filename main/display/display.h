#ifndef DISPLAY_H
#define DISPLAY_H

#include <esp_timer.h>
#include <esp_log.h>

#include <string>

struct DisplayFonts {
    const void* text_font = nullptr;
    const void* icon_font = nullptr;
    const void* emoji_font = nullptr;
};

class Display {
public:
    Display();
    virtual ~Display();

    virtual void SetStatus(const std::string &status);
    virtual void ShowNotification(const std::string &notification, int duration_ms = 3000);
    virtual void SetEmotion(const std::string &emotion);
    virtual void SetChatMessage(const std::string &role, const std::string &content);
    virtual void SetIcon(const char* icon);
    virtual void SetBacklight(uint8_t brightness);

    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline uint8_t brightness() const { return brightness_; }

protected:
    int width_ = 0;
    int height_ = 0;
    uint8_t brightness_ = 0;

    void *display_ = nullptr;

    void *emotion_label_ = nullptr;
    void *network_label_ = nullptr;
    void *status_label_ = nullptr;
    void *notification_label_ = nullptr;
    void *mute_label_ = nullptr;
    void *battery_label_ = nullptr;
    void *chat_message_label_ = nullptr;
    const char* battery_icon_ = nullptr;
    const char* network_icon_ = nullptr;
    bool muted_ = false;

    esp_timer_handle_t notification_timer_ = nullptr;
    esp_timer_handle_t update_timer_ = nullptr;

    friend class DisplayLockGuard;
    virtual bool Lock(int timeout_ms = 0) = 0;
    virtual void Unlock() = 0;

    virtual void Update();
};

class DisplayLockGuard {
public:
    DisplayLockGuard(Display *display) : display_(display) {
        if (!display_->Lock(3000)) {
            ESP_LOGE("Display", "Failed to lock display");
        }
    }
    ~DisplayLockGuard() {
        display_->Unlock();
    }

private:
    Display *display_;
};

#endif