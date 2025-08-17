#include <Arduino.h>
#include <SPI.h>
#include "RF_Sweeper.h"

// --- Pin Definitions ---

// Shared SPI pins for the default VSPI bus on most ESP32s
// PLEASE VERIFY THESE PINS FOR YOUR SEEED XIAO ESP32S3
#define SPI_SCK_PIN  -1 // Use default SCK
#define SPI_MISO_PIN -1 // Use default MISO
#define SPI_MOSI_PIN -1 // Use default MOSI

// Pins for the first NRF24 module
#define NRF1_CE_PIN  1
#define NRF1_CSN_PIN 2

// Pins for the second NRF24 module
#define NRF2_CE_PIN  43
#define NRF2_CSN_PIN 44

// Create an instance of the library for dual-module operation
RFSweeper sweeper(NRF1_CE_PIN, NRF1_CSN_PIN, NRF2_CE_PIN, NRF2_CSN_PIN);

JammerConfig config;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\nDual NRF24 Jammer Example");

  // **FIX: Explicitly initialize the SPI bus**
  // If your pins differ from the board's defaults, specify them here.
  // Otherwise, SPI.begin() is sufficient.
  // SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
  SPI.begin();
  Serial.println("SPI bus initialized.");

  if (sweeper.begin()) {
    Serial.println("Both NRF24 modules initialized successfully.");
  } else {
    Serial.println("Failed to initialize one or both NRF24 modules. Halting.");
    while(1) delay(100);
  }

  // Configure for a CONSTANT_CARRIER attack on the full spectrum
  config.technique = JammingTechnique::CONSTANT_CARRIER;
  
  Serial.println("Starting Full Spectrum Jammer...");
  if (sweeper.startJammer(JammingMode::FULL_SPECTRUM, config)) {
    Serial.printf("Jammer started successfully in %s mode.\n", sweeper.getModeString());
  } else {
    Serial.println("Failed to start jammer.");
  }
}

void loop() {
  sweeper.handleJammer();

  static unsigned long startTime = millis();
  if (sweeper.isJamming() && (millis() - startTime > 10000)) {
    Serial.printf("Stopping %s jammer...\n", sweeper.getModeString());
    sweeper.stopJammer();
    delay(2000);
    
    Serial.println("Restarting in BLE Advertising mode with Noise Injection...");
    config.technique = JammingTechnique::NOISE_INJECTION;
    sweeper.startJammer(JammingMode::BLE_ADVERTISING, config);
    Serial.printf("Jammer started successfully in %s mode.\n", sweeper.getModeString());
    
    startTime = millis();
  }
}