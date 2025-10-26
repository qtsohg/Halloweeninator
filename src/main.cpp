#include <Arduino.h>
#include <WiFi.h>
#include <espmods/core.hpp>
#include <espmods/network.hpp>
#include <espmods/led.hpp>
#include <espmods/audio.hpp>
#include "NetworkConfigHelper.h"

using espmods::core::LogSerial;
using espmods::network::NetWifiOta;
using espmods::network::NetworkConfig;
using espmods::led::LedStrip;
using espmods::audio::AudioDySv5w;
using espmods::audio::PlayState;


NetWifiOta wifiOta_;
LedStrip ledStrip_(LED_PIN, LED_COUNT, LED_BRIGHTNESS);
AudioDySv5w audio_(AUDIO_TX_PIN, AUDIO_RX_PIN);


struct EffectConfig {
  const char *name;         // Friendly label for logging
  uint16_t soundEffectIdx;  // Identifier / filename for the audio clip
  uint16_t lightEffect;  // Identifier for the lighting routine
};

// Configure your available effect combinations here.
static const EffectConfig kEffects[] = {
    {"0001", 1, 2},
    {"0002", 2, 1},
    {"0003", 3, 3},
    {"0004", 4, 4},
    {"0005", 5, 5},
    {"0006", 6, 6},
    {"0007", 7, 7},
    {"0001", 1, 1},
};

static constexpr size_t kEffectCount = 8;

// Adjustable runtime parameters.
static float g_detectionDistanceCm = 60.0f;             // Trigger distance
static constexpr uint32_t kCooldownMinMs = 1000;        // Minimum cooldown after an effect
static constexpr uint32_t kCooldownMaxMs = 5000;       // Maximum cooldown after an effect
static constexpr uint32_t kSensorPollIntervalMs = 500;  // Sensor sampling interval

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------

static unsigned long g_nextTriggerReadyAt = 0;
static unsigned long g_lastSensorSampleAt = 0;
static size_t g_currentEffectIndex = 0;  // For sequential effect playback

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

float readDistanceCm();
void triggerEffect(const EffectConfig &effect);
void scheduleNextTrigger();
void playSoundEffect(uint16_t effectId);
void playLightEffect(uint16_t effectId);
bool tryPlaySoundEffect(uint16_t effectId);  // New function that returns success/failure

// Allow runtime tuning of the detection distance.
void setDetectionDistanceCm(float distanceCm) { g_detectionDistanceCm = distanceCm; }

void setup() {
  LogSerial.begin(115200);
  LogSerial.println("Halloweeninator Starting...");

  ledStrip_.begin();
  ledStrip_.sparkle(0xEEF22F);

  audio_.begin();

  // configure network
  NetworkConfig config = createNetworkConfig();
  
  wifiOta_.begin(config);

  pinMode(ULTRA_TRIG_PIN, OUTPUT);
  pinMode(ULTRA_ECHO_PIN, INPUT);
  digitalWrite(ULTRA_TRIG_PIN, LOW);

  randomSeed(micros());
  g_nextTriggerReadyAt = millis();
}

void loop() {
  const unsigned long now = millis();

  wifiOta_.loop();
  ledStrip_.update();
  audio_.update();

  // Check WiFi connection periodically
  static unsigned long lastWifiCheck = 0;
  if (now - lastWifiCheck > 10000) {  // Check every 10 seconds
    lastWifiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      LogSerial.println("WARNING: WiFi disconnected!");
    } else {
      LogSerial.printf("WiFi OK - RSSI: %d dBm\n", WiFi.RSSI());
    }
  }

  // Print audio play state less frequently
  static unsigned long lastAudioStateLog = 0;
  static PlayState lastLoggedPlayState = PlayState::UNKNOWN;
  if (now - lastAudioStateLog > 2000) {  // Every 2 seconds instead of every poll
    lastAudioStateLog = now;
    PlayState playState = audio_.getPlayState();

    if (playState != lastLoggedPlayState) {
      LogSerial.printf("Audio state changed: %d\n", static_cast<int>(playState));
      lastLoggedPlayState = playState;
    }

    if (lastLoggedPlayState == PlayState::STOP) {
      //LogSerial.printf("Audio is active (state: STOP)\n");
      ledStrip_.off();
    }
  }

  if (now - g_lastSensorSampleAt < kSensorPollIntervalMs || lastLoggedPlayState != PlayState::STOP) {
    delay(5);
    return;
  }

  g_lastSensorSampleAt = now;

  const float distanceCm = readDistanceCm();
 
  if (distanceCm < 0.0f) {
    // No valid reading, try again on the next loop iteration.
    return;
  }
  LogSerial.printf("Distance: %.2f cm\n", distanceCm);

  if (distanceCm <= g_detectionDistanceCm && now >= g_nextTriggerReadyAt) {
    // Check if audio is ready before triggering
    if (lastLoggedPlayState != PlayState::STOP) {
      LogSerial.printf("Audio busy (state: %d), skipping trigger\n", static_cast<int>(lastLoggedPlayState));
      return;
    }
    
    // Sequential effect selection for debugging
    const size_t effectIndex = g_currentEffectIndex;
    //g_currentEffectIndex = (g_currentEffectIndex + 1) % kEffectCount;
    
    const EffectConfig &effect = kEffects[effectIndex];

    LogSerial.printf("Target detected at %.2f cm. Triggering effect: %s (index %zu)\n", distanceCm,
                  effect.name, effectIndex);
    
    bool effectSucceeded = true; // tryPlaySoundEffect(effect.soundEffectIdx);
    audio_.playByFilename(effect.name);
    if (effectSucceeded) {
      playLightEffect(effect.lightEffect);
      scheduleNextTrigger();
      g_currentEffectIndex = (g_currentEffectIndex + 1) % kEffectCount;
    } else {
      LogSerial.println("Effect failed, not scheduling cooldown - can retry immediately");
      // Don't schedule next trigger, so it can be tried again right away
    }
  }
    
}

// -----------------------------------------------------------------------------
// Helper implementations
// -----------------------------------------------------------------------------

float readDistanceCm() {
  // Send a 10µs pulse to start measurement.
  digitalWrite(ULTRA_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRA_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRA_TRIG_PIN, LOW);

  // Listen for the echo; timeout after 30ms (~5m distance) to avoid stalling.
  const unsigned long duration = pulseIn(ULTRA_ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) {
    LogSerial.println("Distance measurement timed out");
    return -1.0f;  // Timeout
  }

  // Convert the round-trip time to distance (speed of sound ~343 m/s).
  const float distanceCm = duration * 0.0343f / 2.0f;
  return distanceCm;
}

void triggerEffect(const EffectConfig &effect) {
  bool audioStarted = tryPlaySoundEffect(effect.soundEffectIdx);
  if (audioStarted) {
    playLightEffect(effect.lightEffect);
  } else {
    LogSerial.println("Audio failed to start, skipping light effect");
  }
}

void scheduleNextTrigger() {
  const uint32_t cooldownDelay = static_cast<uint32_t>(
      random(static_cast<long>(kCooldownMinMs), static_cast<long>(kCooldownMaxMs) + 1));
  g_nextTriggerReadyAt = millis() + cooldownDelay;
  LogSerial.printf("Next trigger available in %lu ms\n", static_cast<unsigned long>(cooldownDelay));
}

// -----------------------------------------------------------------------------
// Hardware integration stubs
// -----------------------------------------------------------------------------

bool tryPlaySoundEffect(uint16_t effectId) {
  LogSerial.printf("  -> Attempting to play sound effect ID: %u\n", effectId);
  
  // Simple approach: just send the play command
  audio_.play(effectId);
  
  // Wait half a second then check if it's playing
  delay(500);
  
  PlayState currentState = audio_.getPlayState();
  LogSerial.printf("  -> Audio state after 500ms: %d\n", static_cast<int>(currentState));
  
  if (currentState == PlayState::PLAY) {
    LogSerial.println("  -> Audio playback confirmed");
    return true;
  } else {
    LogSerial.println("  -> Audio playback failed");
    return false;
  }
}

void playSoundEffect(uint16_t effectId) {
  LogSerial.printf("  -> Attempting to play sound effect ID: %u\n", effectId);
  
  
  // Additional delay to ensure module is ready
  delay(100);
  
  // Now send the play command
  LogSerial.println("  -> Sending play command");
  audio_.play(effectId);
  
  // Wait for the play command to be processed and confirmed
  uint32_t playStartTime = millis();
  bool playConfirmed = false;

  delay(500);
  
  PlayState currentState = audio_.getPlayState();
  LogSerial.printf("  -> Checking play state: %d\n", static_cast<int>(currentState));
  if (currentState == PlayState::PLAY) {
    playConfirmed = true;
    LogSerial.println("  -> Playback confirmed started");
  }

  if (!playConfirmed) {
    LogSerial.println("  -> Playback did not start, attempting one retry");
    
    // Wait a bit longer before retrying
    delay(200);
    
    // Try once more
    audio_.play(effectId);
    delay(300);
    
    PlayState finalState = audio_.getPlayState();
    LogSerial.printf("  -> Final audio state: %d\n", static_cast<int>(finalState));
    
    if (finalState != PlayState::PLAY) {
      LogSerial.println("  -> ERROR: Audio playback failed completely!");
    }
  }
}

void playLightEffect(uint16_t effectId) {
  // TODO: Integrate with your lighting controller (e.g., NeoPixels, relays).
  LogSerial.printf("  -> Activating light effect: %u\n", effectId);
  switch (effectId)
  {
  case 1:
    /* code */
    ledStrip_.lightning(0xFF4400);
    break;
  case 2:
    /* code */
    ledStrip_.fire(150);
    break;
  case 3:
    /* code */
    ledStrip_.rainbow(3000);
    break;
  case 4:
    /* code */
    ledStrip_.sparkle(0x00FF00, 20);
    break;
  case 5:
    /* code */
    ledStrip_.strobe(0xFFFFFF, 100);
    break;
  case 6:
    /* code */
    ledStrip_.pulseColor(0x0000FF, 2000);
    break;
  case 7:
    /* code */
    ledStrip_.gradientPulse(0xFF00FF, 0x00FFFF, 3000);
    break;  
  default:
    break;
  }
  
}

