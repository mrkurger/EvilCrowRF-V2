# EvilCrow RF v2 - Enhanced Version with SD Card Fix

## 🚀 Major Updates & New Features

### ✅ **CRITICAL FIX: SD Card Issues Resolved**

- **FIXED**: SD card initialization failures that caused the device to stop recognizing cards
- **FIXED**: SPI bus conflicts between CC1101 and SD card
- **FIXED**: Changed CS pin from 22 to 5 to avoid hardware conflicts
- **NEW**: Advanced SD Card Manager with automatic recovery
- **NEW**: Multiple initialization attempts with progressive delays
- **IMPROVED**: Better error reporting and diagnostics

### 📺 **NEW: Universal TV Remote Control**

- **Turn off TVs instantly** with hardware button or web interface
- **Support for major brands**: Samsung, LG, Sony, Panasonic, Philips, Sharp, Toshiba
- **API endpoints** for remote control integration
- **Scan mode** to turn off all TVs in range

### 🔍 **NEW: Advanced RF Scanner & Spectrum Analyzer**

- **Real-time spectrum analysis** with intelligent signal detection
- **Protocol identification** for garage doors, car keys, and more
- **Frequency band scanning** for ISM bands and common protocols
- **Signal recording and replay** capabilities

### 🌐 **NEW: Enhanced Responsive Web Interface**

- **Modern glassmorphism design** with smooth animations
- **Mobile-optimized** touch controls for phones and tablets
- **Real-time status updates** with live system monitoring
- **Progressive Web App (PWA)** support
- **Advanced file browser** with upload/download capabilities

This enhanced version provides a stable, feature-rich foundation with critical SD card fixes and exciting new capabilities.

## Features

- ✅ **Enhanced CC1101 Radio Control**: Dual module support with advanced configuration
- ✅ **Real-time Dashboard**: Modern web interface with live status updates
- ✅ **Advanced Signal Processing**: Enhanced capture, analysis, and transmission
- ✅ **Multiple Transmission Modes**: Raw data, binary data, and Tesla signals
- ✅ **RF Jammer**: Configurable frequency jamming with duration control
- ✅ **JSON API**: RESTful endpoints for system status and signal data
- ✅ **Auto-receive Management**: Intelligent receive timeout and management
- ✅ **Signal Logging**: Automatic logging to SD card with analysis
- ✅ **Module Switching**: Dynamic switching between CC1101 modules
- ✅ **Memory Monitoring**: Real-time heap usage and system diagnostics
- ✅ **OTA Updates**: Remote firmware update capability
- ✅ **Responsive Design**: Mobile-friendly interface with modern UI

## File Structure

```
firmware/new/
├── EvilCrow-RFv2/
│   ├── EvilCrow-RFv2.ino          # Main firmware file
│   ├── ELECHOUSE_CC1101_SRC_DRV.h # CC1101 driver header
│   └── ELECHOUSE_CC1101_SRC_DRV.cpp # CC1101 driver implementation
├── SD/
│   ├── HTML/                       # Web interface files
│   │   ├── index.html             # Main page
│   │   ├── style.css              # Styling
│   │   ├── javascript.js          # Frontend logic
│   │   ├── rxconfig.html          # RX configuration
│   │   ├── txconfig.html          # TX configuration
│   │   ├── txbinary.html          # Binary transmission
│   │   ├── txtesla.html           # Tesla signal transmission
│   │   └── jammer.html            # Jammer functionality
│   └── URH/
│       └── exampleproto.xml       # Example protocol file
└── README.md                       # This file
```

## Hardware Requirements

- ESP32 development board
- CC1101 RF transceiver module
- MicroSD card (formatted as FAT32)
- Push buttons on pins 0 and 35
- LED on pin 4

## Pin Configuration

- **CC1101 Module 1**: SPI pins 14, 12, 13, 5
- **CC1101 Module 2**: SPI pins 14, 12, 13, 27
- **SD Card**: SPI pins 18, 19, 23, 22
- **Push Button 1**: Pin 0 (with internal pullup)
- **Push Button 2**: Pin 35 (with internal pullup)
- **Status LED**: Pin 4

## Installation

### 1. Arduino IDE Setup

1. Install ESP32 board support in Arduino IDE
2. Install required libraries:
   - AsyncTCP
   - ESPAsyncWebSrv
   - ElegantOTA
   - ESP32-targz

### 2. SD Card Setup

1. Format SD card as FAT32
2. Copy the entire `SD/` folder contents to the root of the SD card
3. Insert SD card into the EvilCrow RF device

### 3. Firmware Upload

1. Open `EvilCrow-RFv2.ino` in Arduino IDE
2. Select ESP32 board and appropriate port
3. Compile and upload the firmware

## Usage

### 1. Initial Setup

1. Power on the device
2. Connect to WiFi network "EvilCrow-RF" (password: 123456789)
3. Open browser and navigate to `192.168.4.1`

### 2. Web Interface

- **Main Page**: Overview and system status
- **RX Config**: Configure receiver settings
- **TX Config**: Configure transmitter settings
- **TX Binary**: Send binary data
- **TX Tesla**: Tesla-specific signal transmission
- **Jammer**: RF jamming functionality

### 3. Enhanced API Endpoints

#### 📺 TV Remote Control (NEW)

- `GET /api/tv/turnoff` - Turn off TVs (optional: ?brand=samsung)
- `GET /api/tv/turnon` - Turn on TVs (optional: ?brand=lg)
- `GET /api/tv/scan` - Scan and turn off all TVs in range
- `GET /api/tv/brands` - Get list of supported TV brands

#### 🔍 RF Scanner & Analysis (NEW)

- `GET /api/scan/start` - Start RF scan (params: start, end, step)
- `GET /api/scan/stop` - Stop current scan
- `GET /api/scan/status` - Get scan progress and status
- `GET /api/scan/results` - Get detected signals (JSON)
- `GET /api/signal/analyze` - Advanced signal analysis
- `GET /api/signal/spectrum` - Spectrum analysis data

#### System Status

- `GET /api/status` - Complete system status (JSON)
- `GET /api/signals` - Signal data and samples (JSON)
- `GET /status` - Basic status (text)

#### RF Configuration

- `POST /setrx` - Configure receiver (frequency, module, mod, deviation, datarate, setrxbw)
- `POST /settx` - Configure transmitter (frequency, module, rawdata, transmissions)

#### Signal Transmission

- `POST /settxbinary` - Transmit binary data (frequency, binarydata, samplepulse)
- `POST /settxtesla` - Transmit Tesla signal (frequency)

#### Jammer Control

- `POST /startjammer` - Start RF jammer (frequency, duration)
- `POST /stopjammer` - Stop RF jammer

#### 📁 File Management (NEW)

- `GET /files` - Enhanced file browser interface
- `GET /download?file=filename` - Download files from SD card
- `POST /upload` - Upload files to SD card

## Default Settings

- **Frequency**: 433.92 MHz
- **Modulation**: ASK/OOK (mod = 2)
- **Deviation**: 47.60 kHz
- **Data Rate**: 99 kBaud
- **RX Bandwidth**: 812 kHz

## Compilation Status

✅ **Compiles successfully** without errors or warnings
✅ **All dependencies resolved**
✅ **Memory usage optimized**

## 🎯 Usage Examples

### Turn Off All TVs in Range

```bash
# Using curl
curl http://192.168.4.1/api/tv/scan

# Using browser
http://192.168.4.1/api/tv/scan
```

### Turn Off Specific TV Brand

```bash
curl http://192.168.4.1/api/tv/turnoff?brand=samsung
```

### Start RF Frequency Scan

```bash
curl "http://192.168.4.1/api/scan/start?start=433&end=434&step=0.1"
```

### Hardware Button Controls

- **Button 1 (Pin 0)**: Quick TV scan - turns off all TVs in range
- **Button 2 (Pin 35)**: Toggle RF receive mode

## Troubleshooting

### 🔧 SD Card Issues (FIXED in this version)

**Previous Problem**: SD card would work initially but stop being recognized after firmware updates.

**Solution Applied**:

1. **Changed CS pin** from 22 to 5 to avoid SPI conflicts
2. **Multiple initialization attempts** with progressive delays
3. **Automatic recovery** when SD card access fails
4. **Better error reporting** in serial monitor

**If you still have SD card issues**:

1. **Format as FAT32** with MBR partition table (not GPT)
2. **Try a different SD card** - some cards are more compatible
3. **Check connections** - ensure CS pin is connected to pin 5
4. **Monitor serial output** for detailed error messages

### 📺 TV Remote Issues

1. **TVs not responding**:

   - Try different brands with `?brand=samsung` parameter
   - Ensure you're within range (typically 5-10 meters)
   - Check that CC1101 is properly initialized

2. **Limited range**:
   - Increase transmission power in settings
   - Check antenna connection
   - Try different frequencies for your region

### 🔍 RF Scanner Issues

1. **No signals detected**:

   - Check RSSI threshold settings
   - Ensure CC1101 is working properly
   - Try scanning known active frequencies

2. **Scanner not starting**:
   - Verify CC1101 initialization
   - Check frequency range parameters
   - Monitor serial output for errors

### Common Issues

1. **SD Card not detected**: Use the new SD Card Manager - it will retry automatically
2. **WiFi connection issues**: Reset device and try again
3. **Web interface not loading**: Check if SD card files are present in /HTML/ folder
4. **CC1101 not working**: Check SPI connections and power supply

### Serial Monitor

Enable serial monitor at 115200 baud to see:

- SD card initialization progress
- TV remote control status
- RF scanner results
- System diagnostics and error messages

## Development Notes

This minimal version provides a stable foundation for:

- Adding advanced attack modules
- Implementing additional protocols
- Extending web interface functionality
- Adding new RF signal processing features

## License

This project maintains the same license as the original EvilCrow RF project.
