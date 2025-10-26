#pragma once

#include <Arduino.h>
#include "secrets.h"

// Device identity
constexpr const char *DEVICE_HOSTNAME = "Halloweeninator";
constexpr const char *DEVICE_ID = "halloweeninator";

// Led constants
constexpr uint8_t LED_COUNT = 119;
constexpr uint8_t LED_BRIGHTNESS = 250; // out of 255

// Pins
constexpr uint8_t LED_PIN = 5;
constexpr uint8_t BUTTON_TOUCH_PIN = 27;
constexpr uint8_t BUTTON_MAINT_PIN = 13;
constexpr uint8_t ULTRA_TRIG_PIN = 21;
constexpr uint8_t ULTRA_ECHO_PIN  = 22;
constexpr uint8_t I2S_BCLK_PIN = 26;
constexpr uint8_t I2S_LRCLK_PIN = 25;
constexpr uint8_t I2S_DATA_PIN = 33;
constexpr uint8_t AUDIO_TX_PIN = 23;
constexpr uint8_t AUDIO_RX_PIN = 34;

// DY-SV5W track ranges
constexpr uint16_t TRACK_COUNT = 3;

