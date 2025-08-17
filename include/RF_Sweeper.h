#ifndef RF_SWEEPER_H
#define RF_SWEEPER_H

#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <vector>

#define SPECTRUM_CHANNELS 80

// Enum for different jammer modes
enum class JammingMode {
    IDLE,
    WIFI,
    BLE_ADVERTISING,
    BLUETOOTH,
    USB_WIRELESS,
    VIDEO_STREAM,
    RC_TOYS,
    FULL_SPECTRUM,
    SINGLE_CHANNEL,
    CHANNEL_FLOOD_CUSTOM // For advanced use with custom channel list
};

enum class JammingTechnique {
    CONSTANT_CARRIER,
    NOISE_INJECTION
};

struct JammerConfig {
    JammingTechnique technique = JammingTechnique::CONSTANT_CARRIER;
    std::vector<int> customChannels;
};

class RFSweeper {
public:
    // Constructor for single module
    RFSweeper(uint8_t ce1, uint8_t csn1);
    // Constructor for dual modules
    RFSweeper(uint8_t ce1, uint8_t csn1, uint8_t ce2, uint8_t csn2);

    ~RFSweeper();

    // Initialization
    bool begin(SPIClass* spi = &SPI);

    // Spectrum Analyzer (operates on the first radio)
    uint8_t* scanSpectrum();
    
    // Jammer
    bool startJammer(JammingMode mode, JammerConfig config = {});
    // **FIX: Add overloaded function for simple single-channel jamming**
    bool startJammer(JammingMode mode, uint8_t channel); 
    void stopJammer();
    void handleJammer(); // Non-blocking, call in loop()
    bool isJamming() const;
    const char* getModeString() const;

private:
    RF24* _radio1;
    RF24* _radio2;
    SPIClass* _spi;
    uint8_t _cePin1, _csnPin1;
    uint8_t _cePin2, _csnPin2;
    bool _isDualMode;

    // Spectrum data
    uint8_t _spectrumData[SPECTRUM_CHANNELS];

    // Jammer state
    bool _isJamming;
    JammingMode _currentMode;
    JammerConfig _currentConfig;
    uint32_t _channelHopIndex;
    uint8_t _singleChannelTarget;

    void jamWithNoise(int channel1, int channel2);
    void jamWithConstantCarrier(int channel1, int channel2);

    const byte* _getCurrentChannelSet();
    size_t _getCurrentChannelSetSize();
};

#endif // RF_SWEEPER_H