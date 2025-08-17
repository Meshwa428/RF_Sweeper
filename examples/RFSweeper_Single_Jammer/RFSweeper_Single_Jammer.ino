#include <RF_Sweeper.h>

// Define your CE and CSN pins for the NRF24L01+ module
#define CE_PIN 4
#define CSN_PIN 5

RFSweeper sweeper(CE_PIN, CSN_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println("NRF24 Jammer");
  Serial.println("Send a character to change mode:");
  Serial.println("  w - Wi-Fi Jammer");
  Serial.println("  b - BLE Advertising Jammer");
  Serial.println("  f - Full Spectrum Jammer");
  Serial.println("  c - Single Channel Jammer (Channel 45)");
  Serial.println("  s - Stop Jammer");

  if (sweeper.begin()) {
    Serial.println("NRF24L01+ module initialized successfully.");
  } else {
    Serial.println("Failed to initialize NRF24L01+ module. Halting.");
    while(1) delay(100);
  }

  Serial.println("Starting Wi-Fi Jammer by default...");
  sweeper.startJammer(JammerMode::WIFI);
}

void loop() {
  // The handleJammer() function must be called in the main loop
  // to manage channel hopping for the jammer.
  sweeper.handleJammer();

  // Check for serial input to change jammer mode
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Stop any current jamming before starting a new mode
    sweeper.stopJammer();
    delay(10); // Short delay to allow module to reset

    switch(cmd) {
      case 'w':
        Serial.println("Starting Wi-Fi Jammer.");
        sweeper.startJammer(JammerMode::WIFI);
        break;
      case 'b':
        Serial.println("Starting BLE Advertising Jammer.");
        sweeper.startJammer(JammerMode::BLE_ADVERTISING);
        break;
      case 'f':
        Serial.println("Starting Full Spectrum Jammer.");
        sweeper.startJammer(JammerMode::FULL_SPECTRUM);
        break;
      case 'c':
        Serial.println("Starting Single Channel Jammer on channel 45.");
        sweeper.startJammer(JammerMode::SINGLE_CHANNEL, 45);
        break;
      case 's':
        Serial.println("Stopping jammer.");
        // The stop command was already issued above.
        break;
      default:
        Serial.println("Unknown command.");
        break;
    }
  }
}