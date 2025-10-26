#include <Arduino.h>

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#ifndef ULTRA_TRIG_PIN
#define ULTRA_TRIG_PIN 23
#endif

#ifndef ULTRA_ECHO_PIN
#define ULTRA_ECHO_PIN 22
#endif

struct EffectConfig {
  const char *name;         // Friendly label for logging
  const char *soundEffect;  // Identifier / filename for the audio clip
  const char *lightEffect;  // Identifier for the lighting routine
};

// Configure your available effect combinations here.
static const EffectConfig kEffects[] = {
    {"Creepy Laugh", "track_1.mp3", "purple_fade"},
    {"Ghoul Growl", "track_2.mp3", "strobe_white"},
    {"Witch Cackle", "track_3.mp3", "orange_wave"},
};

static constexpr size_t kEffectCount = sizeof(kEffects) / sizeof(kEffects[0]);

// Adjustable runtime parameters.
static float g_detectionDistanceCm = 30.0f;             // Trigger distance
static constexpr uint32_t kCooldownMinMs = 5000;        // Minimum cooldown after an effect
static constexpr uint32_t kCooldownMaxMs = 15000;       // Maximum cooldown after an effect
static constexpr uint32_t kSensorPollIntervalMs = 100;  // Sensor sampling interval

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------

static unsigned long g_nextTriggerReadyAt = 0;
static unsigned long g_lastSensorSampleAt = 0;

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

float readDistanceCm();
void triggerEffect(const EffectConfig &effect);
void scheduleNextTrigger();
void playSoundEffect(const char *effectId);
void playLightEffect(const char *effectId);

// Allow runtime tuning of the detection distance.
void setDetectionDistanceCm(float distanceCm) { g_detectionDistanceCm = distanceCm; }

void setup() {
  Serial.begin(115200);
  Serial.println("Halloweeninator Starting...");

  pinMode(ULTRA_TRIG_PIN, OUTPUT);
  pinMode(ULTRA_ECHO_PIN, INPUT);
  digitalWrite(ULTRA_TRIG_PIN, LOW);

  randomSeed(micros());
  g_nextTriggerReadyAt = millis();
}

void loop() {
  const unsigned long now = millis();

  if (now - g_lastSensorSampleAt < kSensorPollIntervalMs) {
    delay(5);
    return;
  }

  g_lastSensorSampleAt = now;

  const float distanceCm = readDistanceCm();
  if (distanceCm < 0.0f) {
    // No valid reading, try again on the next loop iteration.
    return;
  }

  if (distanceCm <= g_detectionDistanceCm && now >= g_nextTriggerReadyAt) {
    const size_t effectIndex = static_cast<size_t>(random(static_cast<long>(kEffectCount)));
    const EffectConfig &effect = kEffects[effectIndex];

    Serial.printf("Target detected at %.2f cm. Triggering effect: %s\n", distanceCm,
                  effect.name);
    triggerEffect(effect);
    scheduleNextTrigger();
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
    return -1.0f;  // Timeout
  }

  // Convert the round-trip time to distance (speed of sound ~343 m/s).
  const float distanceCm = duration * 0.0343f / 2.0f;
  return distanceCm;
}

void triggerEffect(const EffectConfig &effect) {
  playSoundEffect(effect.soundEffect);
  playLightEffect(effect.lightEffect);
}

void scheduleNextTrigger() {
  const uint32_t cooldownDelay = static_cast<uint32_t>(
      random(static_cast<long>(kCooldownMinMs), static_cast<long>(kCooldownMaxMs) + 1));
  g_nextTriggerReadyAt = millis() + cooldownDelay;
  Serial.printf("Next trigger available in %lu ms\n", static_cast<unsigned long>(cooldownDelay));
}

// -----------------------------------------------------------------------------
// Hardware integration stubs
// -----------------------------------------------------------------------------

void playSoundEffect(const char *effectId) {
  // TODO: Integrate with your audio playback hardware (e.g., DFPlayer, I2S).
  Serial.printf("  -> Playing sound effect: %s\n", effectId);
}

void playLightEffect(const char *effectId) {
  // TODO: Integrate with your lighting controller (e.g., NeoPixels, relays).
  Serial.printf("  -> Activating light effect: %s\n", effectId);
}

