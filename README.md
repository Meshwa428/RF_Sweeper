# RF-Sweeper Library for ESP32

This library provides functionalities for NRF24L01+ modules on the ESP32, including spectrum analysis and signal jamming with both single and dual-module configurations. It is extracted and refactored from the Bruce project to be a standalone, hardware-agnostic library.

The name "RF-Sweeper" reflects its dual capabilities: sweeping the spectrum for analysis and "sweeping away" signals with jamming.

## Features

- **Spectrum Analyzer**: Scan the 2.4GHz band and get signal strength data for 80 channels.
- **Signal Jammer**: Broadcast noise or a constant carrier wave on various 2.4GHz protocols.
- **Dual Module Support**: Orchestrate two NRF24 modules for advanced jamming techniques like spectrum bracketing.
- **Multiple Jamming Modes**: Target specific protocols like Wi-Fi, Bluetooth, or jam the full spectrum.

## Installation

Place the `RFSweeper` folder in your PlatformIO project's `lib/` directory. PlatformIO will automatically detect and use it.

## Dependencies

This library depends on the `RF24` library. Add it to your `platformio.ini`:

```ini
lib_deps =
    nrf24/RF24@^1.4.11
```

## Usage

See the `examples` directory for detailed usage.

### Dual-Module Jammer Example

```cpp
#include <RF_Sweeper.h>

#define NRF1_CE_PIN  26
#define NRF1_CSN_PIN 27
#define NRF2_CE_PIN  32
#define NRF2_CSN_PIN 33

RFSweeper sweeper(NRF1_CE_PIN, NRF1_CSN_PIN, NRF2_CE_PIN, NRF2_CSN_PIN);

void setup() {
  Serial.begin(115200);
  if (!sweeper.begin()) {
    Serial.println("Failed to initialize one or both NRF24 modules.");
    while(1);
  }
  
  JammerConfig config;
  config.technique = JammingTechnique::NOISE_INJECTION;
  sweeper.startJammer(JammingMode::WIFI_NARROWBAND, config);
}

void loop() {
  sweeper.handleJammer();
}
```