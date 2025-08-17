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

  Serial.println("NRF24 Spectrum Analyzer");

  if (sweeper.begin()) {
    Serial.println("NRF24L01+ module initialized successfully.");
  } else {
    Serial.println("Failed to initialize NRF24L01+ module. Halting.");
    while(1) delay(100);
  }
}

void loop() {
  // Scan the spectrum and get the data
  uint8_t* spectrumData = sweeper.scanSpectrum();

  // Print the raw data to the Serial Monitor
  Serial.print("Spectrum Data: [");
  for (int i = 0; i < SPECTRUM_CHANNELS; i++) {
    Serial.print(spectrumData[i]);
    if (i < SPECTRUM_CHANNELS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
  
  // You can use this data to create a visualization on a display
  // or for any other kind of processing.

  delay(100); // Adjust scan frequency
}