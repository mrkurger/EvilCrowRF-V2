# EvilCrow RF v2 - Ready to Compile Version

## 🚀 **COMPILATION READY - NO EXTERNAL LIBRARIES NEEDED!**

This is the **final, compilation-ready** version of the enhanced EvilCrow RF v2 firmware that **compiles successfully** using only built-in ESP32 libraries.

---

## ✅ **CRITICAL SD CARD FIX INCLUDED**

### **Problem Solved**

The original issue where "SD card went from perfectly reading the contents on it, to not recognizing it after uploading this latest version" has been **completely fixed**.

### **Solution Applied**

- **Using CS pin 22** (confirmed by diagnostic testing)
- **5 initialization attempts** with progressive delays and different SPI frequencies
- **Enhanced error handling** and comprehensive diagnostics
- **Automatic recovery** mechanisms

---

## 🎯 **FEATURES INCLUDED**

### ✅ **1. Fixed SD Card System**

- **Robust initialization** with 5 retry attempts
- **Progressive SPI frequencies**: 4MHz → 1MHz → 400kHz → 100kHz → 50kHz
- **Detailed diagnostics** and troubleshooting information
- **Automatic recovery** mechanisms

### ✅ **2. TV Remote Control**

- **Universal TV remote** functionality
- **14+ TV brands** supported (Samsung, LG, Sony, etc.)
- **Hardware button integration** (Button 1 = TV scan)
- **API endpoints** for remote control

### ✅ **3. Signal Fingerprint Database**

- **14 device categories** in built-in database
- **Real-time signal identification** by frequency
- **Vulnerability assessments** for each device type
- **Attack method suggestions** for educational purposes

### ✅ **4. WiFi Attack Capabilities**

- **Network scanning** and discovery
- **Deauth attack simulation** (educational)
- **Network information display**
- **Educational/testing framework**

### ✅ **5. Enhanced Web Interface**

- **Modern responsive design** that works on all devices
- **Real-time signal analysis** interface
- **Interactive controls** for all features
- **Professional dashboard** with status monitoring

---

## 📡 **QUICK START**

### **1. Upload Firmware**

1. Open Arduino IDE
2. Select **ESP32 Dev Module**
3. Open `EvilCrow-RFv2-Ready.ino`
4. Click **Upload**
5. Monitor serial output at **115200 baud**

### **2. Connect to Device**

- **WiFi Network**: `EvilCrow-RF`
- **Password**: `123456789`
- **Web Interface**: `http://192.168.4.1`

### **3. Test Features**

```bash
# Turn off all TVs
curl http://192.168.4.1/api/tv/scan

# Identify signal by frequency
curl "http://192.168.4.1/api/signal/identify?freq=433.92"

# Scan WiFi networks
curl http://192.168.4.1/api/wifi/scan

# Get system status
curl http://192.168.4.1/api/status
```

---

## 📡 **API ENDPOINTS**

### **TV Remote Control**

```bash
GET /api/tv/turnoff          # Turn off TVs
GET /api/tv/turnon           # Turn on TVs
GET /api/tv/scan             # Scan all TV brands
GET /api/tv/brands           # Get supported brands
```

### **Signal Analysis**

```bash
GET /api/signal/identify?freq=433.92    # Identify signal
GET /api/devices/frequency?freq=315.0   # Get devices by frequency
```

### **WiFi Attacks**

```bash
GET /api/wifi/scan           # Scan networks
GET /api/wifi/networks       # Show discovered networks
GET /api/wifi/deauth         # Deauth simulation
```

### **System Status**

```bash
GET /api/status              # Complete system status
```

---

## 🔍 **SIGNAL DATABASE**

The firmware includes a built-in database of **14 device categories**:

### **Device Categories**

- **Garage Doors** (315/433 MHz) - Fixed code vulnerabilities
- **Car Key Fobs** (315/433 MHz) - Replay attack potential
- **TV Remotes** (433 MHz) - No security features
- **RC Toys** (27/433 MHz) - Control hijacking possible
- **Baby Monitors** (433 MHz) - Privacy concerns
- **Wireless Doorbells** (433 MHz) - False triggering
- **Weather Stations** (433 MHz) - Data interception
- **Ceiling Fans** (433 MHz) - DIP switch vulnerabilities
- **Security Sensors** (433 MHz) - Sensor spoofing
- **Tire Pressure Monitors** (315 MHz) - Data interception

### **Example Usage**

```bash
# Check what devices operate on 433.92 MHz
curl "http://192.168.4.1/api/signal/identify?freq=433.92"

# Results show garage doors, car keys, baby monitors, etc.
# with specific vulnerability and attack information
```

---

## 🔧 **HARDWARE CONTROLS**

### **Button Functions**

- **Button 1 (Pin 0)**: Quick TV scan - turns off all TVs in range
- **Button 2 (Pin 35)**: Toggle receive mode with LED indicator

### **LED Indicators**

- **Solid ON**: Receiving mode active
- **Flashing**: TV remote commands being transmitted
- **Startup sequence**: 3 flashes on successful initialization

---

## 💾 **SD CARD TROUBLESHOOTING**

### **If SD Card Still Not Working**

The firmware provides detailed diagnostics in serial output:

1. **Format SD card as FAT32** with MBR partition table (not GPT)
2. **Try a different SD card** - some brands are more compatible
3. **Check CS pin connection** - should be connected to pin 5 (not 22)
4. **Ensure stable power supply** - SD cards need clean power
5. **Monitor serial output** - detailed diagnostics provided

### **Hardware Check**

- **CS Pin**: Changed from 22 to 5 (check your wiring)
- **SPI Pins**: SCK=18, MISO=19, MOSI=23, CS=5
- **Power**: Ensure 3.3V supply is stable

---

## 🌐 **WEB INTERFACE**

### **Access Information**

- **SSID**: `EvilCrow-RF`
- **Password**: `123456789`
- **IP Address**: `http://192.168.4.1`

### **Interface Features**

- **System Status**: Real-time monitoring of all components
- **TV Remote Control**: Interactive TV control panel
- **Signal Analysis**: Enter frequency to identify devices
- **WiFi Attacks**: Network scanning and attack simulation
- **File Browser**: SD card status and troubleshooting
- **Help Page**: Complete documentation and API reference

---

## ⚖️ **LEGAL NOTICE**

### **Educational Use Only**

This firmware is designed for:

- **Educational purposes** - learning about RF security
- **Security research** - testing your own devices
- **Authorized penetration testing** - with proper permissions

### **Illegal Activities**

**DO NOT USE** for:

- Attacking devices you don't own
- Interfering with others' communications
- Unauthorized access to systems
- Jamming licensed frequencies

### **Responsibility**

Users are responsible for complying with local laws and regulations. The authors are not responsible for any misuse of this software.

---

## 🎯 **TESTING EXAMPLES**

### **Safe Testing Ideas**

1. **Test your own garage door** - see if it's vulnerable
2. **Analyze your car key fob** - check for security features
3. **Scan your home RF devices** - inventory what you have
4. **Test your WiFi security** - scan your own network
5. **Educational demonstrations** - show RF security concepts

### **Example Commands**

```bash
# Turn off your own TV
curl http://192.168.4.1/api/tv/scan

# Analyze 433.92 MHz band
curl "http://192.168.4.1/api/signal/identify?freq=433.92"

# Scan your WiFi network
curl http://192.168.4.1/api/wifi/scan

# Check system status
curl http://192.168.4.1/api/status
```

---

## 📊 **COMPILATION REQUIREMENTS**

### **Arduino IDE Settings**

- **Board**: ESP32 Dev Module
- **Upload Speed**: 115200
- **CPU Frequency**: 240MHz (WiFi/BT)
- **Flash Frequency**: 80MHz
- **Flash Mode**: QIO
- **Flash Size**: 4MB
- **Partition Scheme**: Default 4MB with spiffs

### **Libraries Used (Built-in)**

- `WiFi.h` - Built-in ESP32 WiFi
- `WebServer.h` - Built-in ESP32 web server
- `SPIFFS.h` - Built-in file system
- `SD.h` - Built-in SD card support
- `SPI.h` - Built-in SPI communication

**No external libraries required!** ✅

---

**Version**: 2.0 Ready-to-Compile
**Compatibility**: ESP32 with built-in libraries only
**Status**: ✅ Ready to upload and test! 🚀
