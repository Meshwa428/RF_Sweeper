#include <RF_Sweeper.h>

// Define pins for your two NRF24L01+ modules
#define NRF1_CE_PIN  26
#define NRF1_CSN_PIN 27

#define NRF2_CE_PIN  32
#define NRF2_CSN_PIN 33

// Create an instance of the library for dual-module operation
RFSweeper sweeper(NRF1_CE_PIN, NRF1_CSN_PIN, NRF2_CE_PIN, NRF2_CSN_PIN);

JammerConfig config;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Dual NRF24 Jammer Example");

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
  // The handleJammer() function must be called in the main loop
  // to manage the jamming process, including channel hopping.
  sweeper.handleJammer();

  // You can add your own logic here to stop or change jamming modes.
  // This example will switch modes every 10 seconds.
  static unsigned long startTime = millis();
  if (sweeper.isJamming() && (millis() - startTime > 10000)) {
    Serial.printf("Stopping %s jammer...\n", sweeper.getModeString());
    sweeper.stopJammer();
    delay(2000); // Wait 2 seconds before starting next mode
    
    Serial.println("Restarting in BLE Advertising mode with Noise Injection...");
    config.technique = JammingTechnique::NOISE_INJECTION;
    sweeper.startJammer(JammingMode::BLE_ADVERTISING, config);
    Serial.printf("Jammer started successfully in %s mode.\n", sweeper.getModeString());
    
    startTime = millis(); // Reset timer for the next cycle
  }
}