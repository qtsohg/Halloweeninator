#pragma once

#include <espmods/network.hpp>
#include "config.h"
#include "secrets.h"

/**
 * @brief Create network configuration for the toothbrush timer application
 * @return NetworkConfig structure with application-specific settings
 */
inline espmods::network::NetworkConfig createNetworkConfig() {
  espmods::network::NetworkConfig config;
  
  // WiFi credentials from secrets.h
  config.wifiSsid = WIFI_SSID;
  config.wifiPassword = WIFI_PASS;
  
  // Device identification from config.h
  config.deviceHostname = DEVICE_HOSTNAME;
  
  // Web server port (default 80)
  config.webServerPort = 80;
  
  return config;
}