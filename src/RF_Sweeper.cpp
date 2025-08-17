#include "RF_Sweeper.h"

// All original channel definitions are restored
static const byte wifi_channels[] = {
    6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
    22, 24, 26, 28,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    46, 48, 50, 52,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
};
static const byte ble_adv_channels[] = {1, 2, 3, 25, 26, 27, 79, 80, 81};
static const byte bluetooth_channels[] = {
    2,  4,  6,  8,  10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40,
    42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80
};
static const byte usb_channels[] = {40, 50, 60};
static const byte video_channels[] = {70, 75, 80};
static const byte rc_channels[] = {1, 3, 5, 7};
static const byte full_channels[] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
    85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100
};

// Garbage payload for Noise Injection
static const char jam_text[] = "xxxxxxxxxxxxxxxx";

RFSweeper::RFSweeper(uint8_t ce1, uint8_t csn1) :
    _radio1(new RF24(ce1, csn1, SPI_SPEED_NRF)), _radio2(nullptr),
    _cePin1(ce1), _csnPin1(csn1), _cePin2(0), _csnPin2(0),
    _isDualMode(false), _isJamming(false), _currentMode(JammingMode::IDLE),
    _channelHopIndex(0), _singleChannelTarget(1)
{
    memset(_spectrumData, 0, sizeof(_spectrumData));
}

RFSweeper::RFSweeper(uint8_t ce1, uint8_t csn1, uint8_t ce2, uint8_t csn2) :
    _radio1(new RF24(ce1, csn1, SPI_SPEED_NRF)), _radio2(new RF24(ce2, csn2, SPI_SPEED_NRF)),
    _cePin1(ce1), _csnPin1(csn1), _cePin2(ce2), _csnPin2(csn2),
    _isDualMode(true), _isJamming(false), _currentMode(JammingMode::IDLE),
    _channelHopIndex(0), _singleChannelTarget(1)
{
    memset(_spectrumData, 0, sizeof(_spectrumData));
}

RFSweeper::~RFSweeper() {
    stopJammer();
    _radio1->powerDown();
    delete _radio1;
    if (_radio2) {
        _radio2->powerDown();
        delete _radio2;
    }
}

bool RFSweeper::begin(SPIClass* spi) {
    _spi = spi;
    bool success1 = false;
    bool success2 = true; 

    if (_radio1) {
        success1 = _radio1->begin(_spi);
    }
    
    if (_isDualMode && _radio2) {
        success2 = _radio2->begin(_spi);
    }

    return success1 && success2;
}

uint8_t* RFSweeper::scanSpectrum() {
    if (!_radio1) return _spectrumData;

    _radio1->stopListening();

    for (int i = 0; i < SPECTRUM_CHANNELS; i++) {
        _radio1->setChannel(i);
        _radio1->startListening();
        delayMicroseconds(128);
        _radio1->stopListening();
        
        int rpd = _radio1->testCarrier() ? 200 : 0;
        _spectrumData[i] = (_spectrumData[i] * 3 + rpd) / 4;
    }
    return _spectrumData;
}

bool RFSweeper::startJammer(JammingMode mode, JammerConfig config) {
    if (_isJamming) return false;

    _isJamming = true;
    _currentMode = mode;
    _currentConfig = config;
    _channelHopIndex = 0;

    if (mode == JammingMode::SINGLE_CHANNEL && !config.customChannels.empty()) {
        _singleChannelTarget = config.customChannels[0];
    } else {
        _singleChannelTarget = 1;
    }

    auto setupRadio = [&](RF24* radio) {
        if (!radio) return;
        radio->setPALevel(RF24_PA_MAX);
        if (!radio->setDataRate(RF24_2MBPS)) Serial.println("Fail setting data Rate");
        if (config.technique == JammingTechnique::CONSTANT_CARRIER) {
            radio->startConstCarrier(RF24_PA_MAX, 45);
        } else {
            byte dummy_addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
            radio->openWritingPipe(dummy_addr);
            radio->setPayloadSize(sizeof(jam_text));
        }
    };
    
    setupRadio(_radio1);
    if(_isDualMode) setupRadio(_radio2);
    
    if (_currentMode == JammingMode::SINGLE_CHANNEL) {
        if (_radio1) _radio1->setChannel(_singleChannelTarget);
        if (_radio2) _radio2->setChannel(_singleChannelTarget);
    }

    return true;
}

void RFSweeper::stopJammer() {
    if (!_isJamming) return;
    
    if (_currentConfig.technique == JammingTechnique::CONSTANT_CARRIER) {
        if (_radio1) _radio1->stopConstCarrier();
        if (_radio2) _radio2->stopConstCarrier();
    }
    if (_radio1) _radio1->powerDown();
    if (_radio2) _radio2->powerDown();
    
    _isJamming = false;
    _currentMode = JammingMode::IDLE;
}

void RFSweeper::handleJammer() {
    if (!_isJamming || _currentMode == JammingMode::SINGLE_CHANNEL || _currentMode == JammingMode::IDLE) {
        return;
    }

    const byte* channelSet = _getCurrentChannelSet();
    size_t channelSetSize = _getCurrentChannelSetSize();

    if (channelSetSize > 0) {
        int ch1 = channelSet[_hopIndex];
        int ch2 = _isDualMode ? channelSet[channelSetSize - 1 - _hopIndex] : -1;

        if (_currentConfig.technique == JammingTechnique::NOISE_INJECTION) {
            jamWithNoise(ch1, ch2);
        } else {
            jamWithConstantCarrier(ch1, ch2);
        }

        _hopIndex = (_hopIndex + 1) % channelSetSize;
    }
}

void RFSweeper::jamWithNoise(int channel1, int channel2) {
    if (_radio1 && channel1 >= 0 && channel1 <= 125) {
        _radio1->setChannel(channel1);
        _radio1->writeFast(&jam_text, sizeof(jam_text));
    }
    if (_isDualMode && _radio2 && channel2 >= 0 && channel2 <= 125) {
        _radio2->setChannel(channel2);
        _radio2->writeFast(&jam_text, sizeof(jam_text));
    }
}

void RFSweeper::jamWithConstantCarrier(int channel1, int channel2) {
    if (_radio1 && channel1 >= 0 && channel1 <= 125) {
        _radio1->setChannel(channel1);
    }
    if (_isDualMode && _radio2 && channel2 >= 0 && channel2 <= 125) {
        _radio2->setChannel(channel2);
    }
}

bool RFSweeper::isJamming() const { return _isJamming; }

const char* RFSweeper::getModeString() const {
    switch(_currentMode) {
        case JammingMode::WIFI: return "Wi-Fi";
        case JammingMode::BLE_ADVERTISING: return "BLE Advertising";
        case JammingMode::BLUETOOTH: return "Bluetooth";
        case JammingMode::USB_WIRELESS: return "USB Wireless";
        case JammingMode::VIDEO_STREAM: return "Video Stream";
        case JammingMode::RC_TOYS: return "RC Toys";
        case JammingMode::FULL_SPECTRUM: return "Full Spectrum";
        case JammingMode::SINGLE_CHANNEL: return "Single Channel";
        case JammingMode::CHANNEL_FLOOD_CUSTOM: return "Custom Channels";
        default: return "Idle";
    }
}

const byte* RFSweeper::_getCurrentChannelSet() {
    if (_currentMode == JammingMode::CHANNEL_FLOOD_CUSTOM) return reinterpret_cast<const byte*>(_currentConfig.customChannels.data());
    switch (_currentMode) {
        case JammingMode::WIFI: return wifi_channels;
        case JammingMode::BLE_ADVERTISING: return ble_adv_channels;
        case JammingMode::BLUETOOTH: return bluetooth_channels;
        case JammingMode::USB_WIRELESS: return usb_channels;
        case JammingMode::VIDEO_STREAM: return video_channels;
        case JammingMode::RC_TOYS: return rc_channels;
        case JammingMode::FULL_SPECTRUM: return full_channels;
        default: return nullptr;
    }
}

size_t RFSweeper::_getCurrentChannelSetSize() {
    if (_currentMode == JammingMode::CHANNEL_FLOOD_CUSTOM) return _currentConfig.customChannels.size();
    switch (_currentMode) {
        case JammingMode::WIFI: return sizeof(wifi_channels);
        case JammingMode::BLE_ADVERTISING: return sizeof(ble_adv_channels);
        case JammingMode::BLUETOOTH: return sizeof(bluetooth_channels);
        case JammingMode::USB_WIRELESS: return sizeof(usb_channels);
        case JammingMode::VIDEO_STREAM: return sizeof(video_channels);
        case JammingMode::RC_TOYS: return sizeof(rc_channels);
        case JammingMode::FULL_SPECTRUM: return sizeof(full_channels);
        default: return 0;
    }
}