#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ============================================================
// ESP32-C3 SuperMini
// MAX98357A + INMP441 + ST7789
// ============================================================

#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// Simplex I2S mode (separate TX/RX channels)
#define AUDIO_I2S_METHOD_SIMPLEX

// MAX98357A Speaker (I2S TX)
#define AUDIO_I2S_SPK_GPIO_BCLK  GPIO_NUM_2
#define AUDIO_I2S_SPK_GPIO_LRCK  GPIO_NUM_3
#define AUDIO_I2S_SPK_GPIO_DOUT  GPIO_NUM_4

// INMP441 Microphone (I2S RX)
#define AUDIO_I2S_MIC_GPIO_SCK   GPIO_NUM_5
#define AUDIO_I2S_MIC_GPIO_WS    GPIO_NUM_6
#define AUDIO_I2S_MIC_GPIO_DIN   GPIO_NUM_7

// Buttons
#define BOOT_BUTTON_GPIO        GPIO_NUM_9
#define TOUCH_BUTTON_GPIO       GPIO_NUM_NC
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_NC

// LED (板载LED)
#define BUILTIN_LED_GPIO        GPIO_NUM_8



#endif // _BOARD_CONFIG_H_
