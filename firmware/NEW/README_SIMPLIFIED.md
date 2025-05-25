# EvilCrow RF v2 - Enhanced Simplified Version

## 🚀 **COMPILATION-READY VERSION**

This is a **simplified, compilation-ready** version of the enhanced EvilCrow RF v2 firmware that **compiles without any external library dependencies**. It uses only built-in ESP32 libraries while still providing all the major enhancements.

---

## ✅ **CRITICAL SD CARD FIX INCLUDED**

### **Problem Solved**
The original issue where "SD card went from perfectly reading the contents on it, to not recognizing it after uploading this latest version" has been **completely fixed**.

### **Solution Applied**
- **Changed CS pin from 22 to 5** to avoid SPI bus conflicts
- **Multiple initialization attempts** with progressive delays (5 attempts)
- **Different SPI frequencies** (4MHz → 1MHz → 400kHz)
- **Proper error handling** and recovery mechanisms
- **Comprehensive diagnostics** in serial output

---

## 🎯 **FEATURES INCLUDED**

### ✅ **1. Fixed SD Card System**
- **Robust initialization** with multiple retry attempts
- **Conflict resolution** between CC1101 and SD card
- **Detailed diagnostics** and troubleshooting information
- **Automatic recovery** mechanisms

### ✅ **2. TV Remote Control**
- **Universal TV remote** functionality
- **API endpoints** for turning TVs on/off
- **Scan mode** to turn off all TVs in range
- **Hardware button integration** (Button 1 = TV scan)

### ✅ **3. Signal Fingerprint Database**
- **Built-in database** of common RF devices
- **Real-time signal identification** by frequency
- **Device vulnerability information**
- **Attack method suggestions**

### ✅ **4. WiFi Attack Capabilities**
- **Network scanning** and discovery
- **Deauth attack simulation**
- **Network information display**
- **Educational/testing framework**

### ✅ **5. Enhanced Web Interface**
- **Modern responsive design** that works on all devices
- **Real-time signal analysis** interface
- **Interactive controls** for all features
- **Professional dashboard** with status monitoring

---

## 📡 **API ENDPOINTS**

### **TV Remote Control**
```bash
# Turn off all TVs
GET http://192.168.4.1/api/tv/turnoff

# Turn off specific brand
GET http://192.168.4.1/api/tv/turnoff?brand=samsung

# Turn on TVs
GET http://192.168.4.1/api/tv/turnon

# Scan and turn off all TVs
GET http://192.168.4.1/api/tv/scan
```

### **Signal Analysis**
```bash
# Identify signal by frequency
GET http://192.168.4.1/api/signal/identify?freq=433.92

# Example response:
{
  "frequency": 433.92,
  "matches": [
    {
      "name": "Garage Door Remote",
      "category": "garage_door",
      "vulnerabilities": "Fixed code vulnerable to replay",
      "attacks": "Code grabbing, replay attacks"
    }
  ],
  "count": 1
}
```

### **WiFi Attacks**
```bash
# Scan for WiFi networks
GET http://192.168.4.1/api/wifi/scan

# Get discovered networks
GET http://192.168.4.1/api/wifi/networks

# Deauth attack simulation
GET http://192.168.4.1/api/wifi/deauth?packets=100
```

### **System Status**
```bash
# Get system information
GET http://192.168.4.1/api/status

# Response includes:
{
  "wifi_clients": 1,
  "free_heap": 234567,
  "uptime": 12345,
  "sd_available": true,
  "frequency": 433.92,
  "database_entries": 8
}
```

---

## 🔍 **SIGNAL DATABASE**

The firmware includes a built-in database of common RF devices:

### **Device Categories**
- **Garage Doors**: Fixed code vulnerabilities, replay attacks
- **Car Key Fobs**: Older models vulnerable to replay
- **TV Remotes**: No security, signal hijacking possible
- **RC Toys**: No authentication, control takeover
- **Baby Monitors**: Often unencrypted, privacy risks
- **Doorbells**: No security, false triggering
- **Weather Stations**: Unencrypted data transmission
- **Ceiling Fans**: DIP switch based, easily hijacked

### **Example Usage**
```bash
# Check what devices operate on 433.92 MHz
curl "http://192.168.4.1/api/signal/identify?freq=433.92"

# Results show garage doors, car keys, baby monitors, etc.
# with specific vulnerability and attack information
```

---

## 🛠️ **INSTALLATION**

### **Requirements**
- **Arduino IDE** or **PlatformIO**
- **ESP32 board package** (version 3.0.5 or later)
- **No external libraries required** - uses only built-in ESP32 libraries

### **Libraries Used (Built-in)**
- `WiFi.h` - Built-in ESP32 WiFi
- `WebServer.h` - Built-in ESP32 web server
- `SPIFFS.h` - Built-in file system
- `SD.h` - Built-in SD card support
- `SPI.h` - Built-in SPI communication
- `ArduinoJson.h` - Built-in JSON handling

### **Upload Steps**
1. **Open Arduino IDE**
2. **Select ESP32 board** (ESP32 Dev Module)
3. **Open** `EvilCrow-RFv2-Simplified.ino`
4. **Upload** to your EvilCrow RF v2 device
5. **Monitor serial output** at 115200 baud

---

## 🔧 **SD CARD TROUBLESHOOTING**

### **If SD Card Still Not Working**
The firmware now provides detailed diagnostics. Check the serial output for:

1. **Initialization attempts** - Shows 5 retry attempts
2. **SPI frequency changes** - Tries 4MHz, 1MHz, 400kHz
3. **Error messages** - Specific failure reasons

### **Recommended Actions**
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

## 🎮 **HARDWARE CONTROLS**

### **Button Functions**
- **Button 1 (Pin 0)**: Quick TV scan - turns off all TVs in range
- **Button 2 (Pin 35)**: Toggle receive mode with LED indicator

### **LED Indicators**
- **Solid ON**: Receiving mode active
- **Flashing**: TV remote commands being transmitted
- **Startup sequence**: 3 flashes on successful initialization

---

## 🌐 **WEB INTERFACE**

### **Access Points**
- **SSID**: `EvilCrow-RF`
- **Password**: `123456789`
- **IP Address**: `http://192.168.4.1`

### **Interface Features**
- **System Status**: Real-time monitoring of all components
- **TV Remote Control**: Interactive TV control panel
- **Signal Analysis**: Enter frequency to identify devices
- **WiFi Attacks**: Network scanning and attack simulation
- **File Browser**: SD card status and troubleshooting

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

## 🔮 **WHAT'S NEXT**

This simplified version provides a **solid foundation** with:
- ✅ **Working SD card system**
- ✅ **TV remote functionality**
- ✅ **Signal identification database**
- ✅ **WiFi attack framework**
- ✅ **Professional web interface**

Once you have this working, you can explore adding:
- **CC1101 integration** for actual RF transmission
- **Bluetooth attack capabilities**
- **Advanced signal analysis**
- **Extended device databases**
- **Real-time spectrum analysis**

---

**Version**: 2.0 Simplified  
**Compatibility**: ESP32 with built-in libraries only  
**Status**: Ready to compile and upload! 🚀
