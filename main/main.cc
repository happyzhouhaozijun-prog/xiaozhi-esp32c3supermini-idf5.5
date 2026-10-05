#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>

#include "application.h"
#include "system_info.h"


#include "board.h"           // ← 添加
#include "assets/zh/binary.h" // ← 添加（欢迎音数据）
#include "audio_codec.h"  // ← 添加这行！

#include <esp_wifi.h>  // ← 添加这行


#define TAG "main"





extern "C" void app_main(void)
{

    // ===== 新增：初始化 GPIO8 并闪烁 3 下 =====
    // 配置 GPIO8 为输出模式
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << 8);  // GPIO8
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // 闪烁 3 下
    for (int i = 0; i < 5; i++) {
        gpio_set_level(GPIO_NUM_8, 1);  // 高电平
        vTaskDelay(pdMS_TO_TICKS(20)); // 亮 200ms
        gpio_set_level(GPIO_NUM_8, 0);  // 低电平
        vTaskDelay(pdMS_TO_TICKS(100)); // 灭 200ms
    }





    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());// 创建事件循环
    ESP_LOGI(TAG, "1=== after create event_loop ===");
    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();// 初始化Flash存储（WiFi密码等）
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "2=== after init flash , there are wifi id and password . ===");

    // Launch the application
    Application::GetInstance().Start();// 启动主应用

    // ⭐ 新增：降低 Wi-Fi 发射功率，减少断线
    //esp_wifi_set_max_tx_power(8);  // 8dBm 
    
    ESP_LOGI(TAG, "3=== after start application ===");



    // ⭐ 新增：创建定时器任务，10秒后播放欢迎音
    xTaskCreate([](// ← 创建并启动一个独立的FreeRTOS任务
        void* arg) {// ← Lambda表达式：任务的实际函数体
        vTaskDelay(pdMS_TO_TICKS(10000)); // 延时10秒
        
        ESP_LOGI("Startup", "10 seconds passed, playing 1 sound...");
        
        // 获取音频编解码器
        auto& board = Board::GetInstance();
        auto codec = board.GetAudioCodec();
        
        // 播放欢迎音
        codec->EnableOutput(true);
        
        // 播放欢迎音数据（从 binary.h 中获取）
        auto& app = Application::GetInstance();

        app.PlayLocalFile(p3_1_start, p3_1_end - p3_1_start);
       
        ESP_LOGI("Startup", "1 sound played!");
        vTaskDelete(NULL); // ← 任务执行完自己把自己删掉
    }, 
    "1_timer",// ← 任务名称（方便调试）
     4096, // ← 栈大小：4096字节（约4KB）
     NULL, // ← 任务参数（这里没用）
     1, // ← 任务优先级（1是最低，数值越大优先级越高）
     nullptr); // ← 任务句柄（这里不用）

/**/



    // ⭐ 新增：创建定时器任务，10秒后播放欢迎音
    xTaskCreate([](void* arg) {
        vTaskDelay(pdMS_TO_TICKS(12000)); // 延时10秒
        ESP_LOGI("Startup", "12 seconds passed, playing 2 sound...");
        // 获取音频编解码器
        auto& board = Board::GetInstance();
        auto codec = board.GetAudioCodec();
        // 播放欢迎音
        codec->EnableOutput(true);
        // 播放欢迎音数据（从 binary.h 中获取）
        auto& app = Application::GetInstance();
        app.PlayLocalFile(p3_2_start, p3_2_end - p3_2_start);
        ESP_LOGI("Startup", "2 sound played!");
        vTaskDelete(NULL);
    }, "2_timer", 4096, NULL, 1, nullptr);


    // ⭐ 新增：创建定时器任务，10秒后播放欢迎音
    xTaskCreate([](void* arg) {
        vTaskDelay(pdMS_TO_TICKS(14000)); // 延时10秒
        ESP_LOGI("Startup", "14 seconds passed, playing 3 sound...");
        // 获取音频编解码器
        auto& board = Board::GetInstance();
        auto codec = board.GetAudioCodec();
        // 播放欢迎音
        codec->EnableOutput(true);
        // 播放欢迎音数据（从 binary.h 中获取）
        auto& app = Application::GetInstance();
        app.PlayLocalFile(p3_3_start, p3_3_end - p3_3_start);
        ESP_LOGI("Startup", "3 sound played!");
        vTaskDelete(NULL);
    }, "3_timer", 4096, NULL, 1, nullptr);    

/*
*/





    // Dump CPU usage every 10 second// 空闲循环，打印内存信息
    while (true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        // SystemInfo::PrintRealTimeStats(pdMS_TO_TICKS(1000));
        int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        int min_free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGI(TAG, "Free internal: %u minimal internal: %u", free_sram, min_free_sram);
    }
}
