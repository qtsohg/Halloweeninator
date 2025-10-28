# Persistent Configuration System for Halloweeninator

This document explains the new persistent configuration system that allows you to adjust cooldown timings through the web interface with automatic saving to flash memory.

## What's New

### ✅ **Persistent Storage**
- Configuration values are now automatically saved to ESP32's flash memory (NVS)
- Settings persist across reboots and power cycles
- Uses ESP32's built-in Preferences library for reliable storage

### ✅ **Web Interface Controls**
Your web dashboard now has **3 configurable sliders**:

1. **Detection Distance (10-200 cm)**
   - Range: 10cm to 200cm, 1cm steps
   - Default: 60cm
   - Controls how close someone needs to be to trigger effects

2. **Min Cooldown (500-10,000 ms)**
   - Range: 0.5 to 10 seconds, 100ms steps  
   - Default: 1000ms (1 second)
   - Minimum time before another effect can trigger

3. **Max Cooldown (1,000-30,000 ms)**
   - Range: 1 to 30 seconds, 100ms steps
   - Default: 5000ms (5 seconds)  
   - Maximum time before another effect can trigger

### ✅ **Smart Validation**
- If you set Min Cooldown higher than Max Cooldown, Max automatically adjusts up
- If you set Max Cooldown lower than Min Cooldown, Min automatically adjusts down
- All changes are immediately saved to flash memory

## How It Works

### **Automatic Saving**
Every time you move a slider in the web interface:
1. The value updates immediately
2. Gets saved to flash memory automatically  
3. Logs the change to the web console
4. Takes effect immediately (no restart needed)

### **Automatic Loading**
When the ESP32 boots up:
1. Loads all saved values from flash memory
2. Uses defaults if no saved values exist
3. Logs the loaded configuration to console

### **Storage Keys**
Configuration is stored with these keys:
- `detection_distance` - Detection distance in cm
- `cooldown_min` - Minimum cooldown in milliseconds
- `cooldown_max` - Maximum cooldown in milliseconds

## Usage Examples

### **Web Interface**
1. Go to `http://halloweeninator.local/`
2. In the "Controls" section, you'll see 3 sliders
3. Drag any slider to change values
4. Values save automatically (you'll see confirmation in logs)

### **What You'll See in Logs**
```
[12:34:56.123] Loaded config - Detection: 60.0 cm, Cooldown: 1000-5000 ms
[12:35:15.456] Detection Distance → 45.0 cm
[12:35:20.789] Min Cooldown → 2000 ms
[12:35:25.123] Max Cooldown → 8000 ms
[12:35:30.456] Configuration saved to flash
```

### **Cooldown Behavior**
- After each effect triggers, a random cooldown between Min and Max is selected
- Example: Min=2000ms, Max=8000ms → random cooldown between 2-8 seconds
- The actual cooldown chosen is logged: `Next trigger available in 4523 ms`

## Technical Details

### **Storage Implementation**
- Uses ESP32's NVS (Non-Volatile Storage) partition
- Data survives firmware updates, reboots, and power loss
- Stored in namespace "halloweeninator"
- Efficient: only writes when values change

### **Memory Usage**
- Minimal impact: ~100 bytes of flash storage
- No performance impact during normal operation
- Values cached in RAM during operation

### **Backup/Reset**
If you want to reset to defaults:
1. The easiest way is to manually set sliders back to defaults
2. Or reflash firmware to clear all stored settings

### **Configuration File Location**
The persistent storage system is implemented in:
- `lib/OlPie-esp32-modules/src/core/Storage.h` - Storage interface
- `lib/OlPie-esp32-modules/src/core/Storage.cpp` - Storage implementation
- `src/main.cpp` - Configuration loading/saving logic

## Default Values
- **Detection Distance**: 60.0 cm
- **Min Cooldown**: 1000 ms (1 second)
- **Max Cooldown**: 5000 ms (5 seconds)

## Benefits

1. **Remote Configuration**: Adjust settings from anywhere on your network
2. **Persistent Settings**: No need to reconfigure after power cycles
3. **Real-time Feedback**: See changes immediately in logs
4. **Safe Ranges**: Sliders prevent invalid values
5. **Automatic Validation**: Min/Max values stay consistent

Your Halloweeninator is now fully configurable and will remember your settings! 🎃