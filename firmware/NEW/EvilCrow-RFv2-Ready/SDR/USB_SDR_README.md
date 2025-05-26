# 🚀 EvilCrow RF v2 - Complete USB SDR Implementation

## 📡 **Overview**

The EvilCrow RF v2 now functions as a **complete Software Defined Radio (SDR)** when connected to a PC via USB. This implementation provides full compatibility with popular SDR software and includes advanced signal analysis capabilities.

## ✨ **Features**

### 🎯 **Core SDR Functionality**
- ✅ **HackRF Protocol Compatibility** - Works with HackRF-compatible software
- ✅ **GNU Radio Companion Support** - Direct integration with GNU Radio
- ✅ **SDR# (SDRSharp) Compatibility** - Windows SDR software support
- ✅ **Universal Radio Hacker Integration** - Protocol analysis and reverse engineering
- ✅ **Real-time IQ Streaming** - Up to 2 MSPS sample rate
- ✅ **Spectrum Analysis** - Live spectrum display with FFT
- ✅ **Protocol Detection** - Automatic OOK, FSK, ASK detection
- ✅ **Web Interface Control** - Browser-based SDR control panel

### 📊 **Technical Specifications**
- **Frequency Range**: 300-928 MHz (CC1101 limitation)
- **Sample Rates**: 250 kHz, 500 kHz, 1 MHz, 2 MHz
- **Bandwidth**: Up to 325 kHz
- **Gain Range**: 0-30 dB
- **Interface**: USB Serial (115200 baud)
- **Protocols**: Binary IQ, Text format, HackRF compatible

### 🔧 **Software Compatibility**
- 📡 **GNU Radio Companion** - Signal processing flowgraphs
- 📻 **SDR# (SDRSharp)** - Windows spectrum analyzer
- 🔍 **Universal Radio Hacker** - Protocol reverse engineering
- 📊 **CubicSDR** - Cross-platform SDR software
- 🎯 **Gqrx** - Linux SDR receiver
- 🐍 **Python Libraries** - Custom control scripts

## 🚀 **Quick Start Guide**

### 1. **Flash the Firmware**
```bash
# Upload EvilCrow-SDR-Complete.ino to your EvilCrow RF v2
# Use Arduino IDE with ESP32 board support
```

### 2. **Connect via USB**
```bash
# Linux
sudo dmesg | grep tty  # Find your device (usually /dev/ttyUSB0)

# Windows
# Check Device Manager for COM port (usually COM3, COM4, etc.)
```

### 3. **Test Connection**
```bash
# Open serial terminal at 115200 baud
# Send: board_id_read
# Expected response: HACKRF_ONE
```

### 4. **Basic SDR Commands**
```bash
# Set frequency to 433.92 MHz
set_freq 433920000

# Set sample rate to 1 MHz
set_sample_rate 1000000

# Set gain to 20 dB
set_gain 20

# Start receiving
rx_start

# Stop receiving
rx_stop

# Get status
status

# Get help
help
```

## 🐍 **Python Control Library**

### **Installation**
```bash
pip install pyserial numpy matplotlib
```

### **Basic Usage**
```python
from EvilCrowSDR_Python_Library import EvilCrowSDR

# Connect to EvilCrow
sdr = EvilCrowSDR('/dev/ttyUSB0')  # Linux
# sdr = EvilCrowSDR('COM3')        # Windows

# Configure SDR
sdr.set_frequency(433.92e6)  # 433.92 MHz
sdr.set_sample_rate(1e6)     # 1 MHz
sdr.set_gain(20)             # 20 dB

# Start receiving
sdr.start_rx()

# Read samples
samples = sdr.read_samples(1024)
print(f"Received {len(samples)} samples")

# Get spectrum
freqs, power = sdr.get_spectrum()

# Detect protocols
protocols = sdr.detect_protocols()

# Stop and close
sdr.stop_rx()
sdr.close()
```

## 📡 **GNU Radio Integration**

### **Installation**
1. Copy `GNU_Radio_EvilCrow_Example.py` to your GNU Radio blocks directory
2. Add the XML block definition to GNU Radio Companion
3. Restart GNU Radio Companion

### **Usage in GNU Radio**
1. Add "EvilCrow SDR Source" block to your flowgraph
2. Configure serial port, frequency, sample rate, and gain
3. Connect to other GNU Radio blocks (filters, demodulators, etc.)
4. Run the flowgraph

### **Example Flowgraph**
```
EvilCrow SDR Source → Low Pass Filter → FM Demod → Audio Sink
```

## 🌐 **Web Interface**

### **Access**
1. Connect EvilCrow to power
2. Connect to WiFi network: `EvilCrow-SDR` (password: `123456789`)
3. Open browser to: `http://192.168.4.1`

### **Features**
- 📡 Real-time SDR control
- 📊 Live status monitoring
- 📈 Spectrum analyzer display
- 🔧 Frequency and gain adjustment
- 🔍 Protocol detection toggle

## 📻 **SDR Software Setup**

### **SDR# (Windows)**
1. Install SDR#
2. Install ExtIO plugin for serial devices
3. Configure COM port and baud rate (115200)
4. Select "EvilCrow RF v2" as source
5. Set frequency and start

### **GNU Radio (Linux/Windows)**
1. Install GNU Radio 3.8+
2. Copy EvilCrow source block
3. Create flowgraph with EvilCrow source
4. Configure parameters and run

### **Universal Radio Hacker**
1. Install URH
2. Add EvilCrow as custom device
3. Configure serial port
4. Use for protocol analysis

## 🔧 **Command Reference**

### **HackRF Compatible Commands**
```bash
board_id_read              # Get device ID
set_freq <Hz>              # Set center frequency
set_sample_rate <Hz>       # Set sample rate
set_gain <dB>              # Set gain (0-30)
rx_start                   # Start receiving
rx_stop                    # Stop receiving
tx_start                   # Start transmitting
tx_stop                    # Stop transmitting
```

### **GNU Radio Commands**
```bash
gnuradio_get_device_info   # Device information
gnuradio_start_stream      # Start streaming
gnuradio_stop_stream       # Stop streaming
```

### **SDR# Commands**
```bash
sdr_get_device_count       # Get device count
sdr_get_device_name        # Get device name
sdr_open                   # Open device
sdr_close                  # Close device
sdr_set_center_freq <Hz>   # Set frequency
sdr_set_sample_rate <Hz>   # Set sample rate
```

### **Universal Radio Hacker Commands**
```bash
urh_get_device_info        # Device information
urh_start_recording        # Start recording
urh_stop_recording         # Stop recording
urh_get_protocols          # Get detected protocols
```

### **EvilCrow Specific Commands**
```bash
spectrum_start             # Start spectrum analysis
spectrum_stop              # Stop spectrum analysis
protocol_decode            # Enable protocol detection
protocol_stop              # Disable protocol detection
status                     # Show SDR status
help                       # Show command help
```

## 🎯 **Use Cases**

### **Signal Analysis**
- 📡 RF spectrum monitoring
- 🔍 Protocol reverse engineering
- 📊 Signal characterization
- 🎵 Audio signal demodulation

### **Research & Development**
- 🧪 RF protocol development
- 📈 Signal processing research
- 🔬 Wireless communication analysis
- 🎓 Educational projects

### **Security Testing**
- 🔐 RF security assessment
- 📡 Wireless protocol analysis
- 🕵️ Signal intelligence gathering
- 🛡️ RF vulnerability testing

## 🔧 **Troubleshooting**

### **Connection Issues**
```bash
# Check serial port permissions (Linux)
sudo usermod -a -G dialout $USER
sudo chmod 666 /dev/ttyUSB0

# Check device manager (Windows)
# Ensure correct COM port and drivers
```

### **No Response from Device**
```bash
# Reset device
# Check baud rate (115200)
# Verify firmware is flashed correctly
# Try different USB cable/port
```

### **Poor Signal Quality**
```bash
# Adjust gain settings
# Check antenna connection
# Verify frequency range (300-928 MHz)
# Reduce sample rate if needed
```

## 📚 **Additional Resources**

- 📖 **GNU Radio Tutorials**: https://wiki.gnuradio.org/
- 📻 **SDR# Documentation**: https://airspy.com/download/
- 🔍 **Universal Radio Hacker**: https://github.com/jopohl/urh
- 🐍 **Python SDR Examples**: See included library files

## 🎉 **Conclusion**

The EvilCrow RF v2 USB SDR implementation transforms your device into a powerful, versatile Software Defined Radio platform. With compatibility across multiple software packages and comprehensive Python libraries, it's perfect for RF research, signal analysis, and wireless security testing.

**Happy SDR-ing!** 📡🚀
