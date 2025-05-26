# EvilCrow RF v2 - Enhanced SDR Firmware

**Contribution to the original EvilCrow RF v2 project by Joel Serna Moreno**

## 🎯 **What This Adds to EvilCrow**

This enhanced firmware transforms EvilCrow RF v2 into a professional Software Defined Radio (SDR) platform with:

### ✨ **New Capabilities:**
- **URH Integration** - Works with Universal Radio Hacker for professional RF analysis
- **GNU Radio Support** - Compatible with GNU Radio Companion
- **Python SDK** - Complete Python library for automation and custom applications
- **Modern Web Interface** - Professional dashboard with real-time updates
- **HackRF Protocol Compatibility** - Standard SDR command interface

### 🔧 **Technical Improvements:**
- **Proper CC1101 Library Integration** - Uses ELECHOUSE_CC1101_SRC_DRV for reliable operation
- **Real RF Data Processing** - Actual CC1101 FIFO data instead of synthetic samples
- **Stable SPI Communication** - Correct pin configuration for EvilCrow hardware
- **Professional API** - RESTful web API for remote control

## 📁 **Files Included:**

| File | Description |
|------|-------------|
| `ECRFv2-SDR.ino` | Main enhanced firmware with SDR capabilities |
| `ELECHOUSE_CC1101_SRC_DRV.h` | CC1101 library header (locally included) |
| `ELECHOUSE_CC1101_SRC_DRV.cpp` | CC1101 library implementation |
| `urh_compatible_bridge.py` | Python bridge for URH integration |
| `EvilCrowSDR_Python_Library.py` | Complete Python SDK |
| `GNU_Radio_EvilCrow_Example.py` | GNU Radio integration example |
| `web_interface_modern.html` | Standalone modern web interface |

## 🚀 **Installation:**

1. **Upload Firmware**: Load `ECRFv2-SDR.ino` in Arduino IDE
2. **Connect Hardware**: EvilCrow RF v2 with CC1101 module
3. **Access Web Interface**: Connect to "EvilCrow-SDR" WiFi, browse to 192.168.4.1

## 🔗 **Integration Examples:**

### **URH (Universal Radio Hacker):**
```bash
python3 urh_compatible_bridge.py
# In URH: RTL-TCP, 127.0.0.1:1234
```

### **Python SDK:**
```python
from EvilCrowSDR_Python_Library import EvilCrowSDR
sdr = EvilCrowSDR('/dev/ttyUSB0')
sdr.set_frequency(433.92e6)
sdr.start_rx()
```

### **GNU Radio:**
```python
# See GNU_Radio_EvilCrow_Example.py for complete flowgraph
```

## 🎯 **Why This Enhances EvilCrow:**

1. **Professional Tool Integration** - Works with industry-standard RF analysis software
2. **Expanded Use Cases** - Research, education, security testing, protocol analysis
3. **Better Hardware Utilization** - Proper CC1101 library usage for reliable operation
4. **Modern Interface** - Professional web dashboard instead of basic HTML
5. **Automation Ready** - Python SDK enables scripting and automation

## 🤝 **Compatibility:**

- ✅ **Maintains all original EvilCrow functionality**
- ✅ **Uses same hardware (no modifications required)**
- ✅ **Backward compatible with existing workflows**
- ✅ **Adds new capabilities without breaking existing features**

## 📊 **Testing Status:**

- ✅ **CC1101 Communication** - Verified with multiple hardware units
- ✅ **URH Integration** - Successfully tested with Universal Radio Hacker
- ✅ **Web Interface** - Tested on desktop, tablet, and mobile browsers
- ✅ **Python SDK** - Verified automation and scripting capabilities
- ✅ **GNU Radio** - Confirmed flowgraph integration

## 🎉 **Impact:**

This enhancement positions EvilCrow RF v2 as a **professional-grade RF security platform** that can compete with commercial tools costing thousands of dollars, while maintaining the accessibility and affordability that makes EvilCrow special.

## 🙏 **Credits:**

- **Original EvilCrow RF v2**: Joel Serna Moreno
- **Enhanced SDR Implementation**: Community contribution
- **CC1101 Library**: ELECHOUSE
- **Integration Tools**: URH, GNU Radio communities

---

**This contribution aims to expand EvilCrow's capabilities while honoring the original vision of an accessible, powerful RF security platform.**
