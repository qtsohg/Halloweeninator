# Halloweeninator
Halloween Jump Scare device.

## Project Structure

This is a PlatformIO project for ESP32 with the following structure:

- `src/` - Source files
- `include/` - Header files
- `lib/` - Project libraries
  - `OlPie-esp32-modules/` - Git submodule containing OlPie ESP32 modules
- `test/` - Test files
- `platformio.ini` - PlatformIO configuration

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- Git

### Setup

1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/qtsohg/Halloweeninator.git
   ```

   Or if you already cloned without submodules:
   ```bash
   git submodule update --init --recursive
   ```

2. Build the project:
   ```bash
   pio run
   ```

3. Upload to ESP32:
   ```bash
   pio run --target upload
   ```

4. Monitor serial output:
   ```bash
   pio device monitor
   ```
