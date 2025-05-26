# EvilCrow RF v2 - URH Setup Guide

## 🎯 Quick Setup Steps

### Step 1: Test EvilCrow Connection
```bash
# Make scripts executable
chmod +x test_evilcrow.py
chmod +x evilcrow_urh_bridge.py

# Test EvilCrow connection
python3 test_evilcrow.py
```

### Step 2: Start the Bridge
```bash
# Start the URH bridge
python3 evilcrow_urh_bridge.py

# Or with custom parameters
python3 evilcrow_urh_bridge.py -p /dev/cu.usbserial-11440 -t 1234 -f 433920000
```

### Step 3: Configure URH
1. **Open Universal Radio Hacker (URH)**
2. **Go to File → Options → Device tab**
3. **Make sure RTL-TCP is checked** ✅
4. **Close Options dialog**

### Step 4: Connect in URH
1. **In URH main window, click the device dropdown**
2. **Select "RTL-TCP"**
3. **Set connection parameters:**
   - **IP Address**: `127.0.0.1`
   - **Port**: `1234`
   - **Sample Rate**: `2000000` (2 MHz)
   - **Frequency**: `433920000` (433.92 MHz)

### Step 5: Start Recording
1. **Click the "Start" button** in URH
2. **You should see IQ data streaming**
3. **Adjust frequency/gain as needed**

## 🔧 Troubleshooting

### Bridge Won't Start
```bash
# Check if port is in use
lsof | grep usbserial

# Check permissions
ls -la /dev/cu.usbserial-11440

# Kill any existing connections
sudo pkill -f usbserial
```

### URH Can't Connect
```bash
# Test TCP connection manually
telnet 127.0.0.1 1234

# Check if bridge is listening
netstat -an | grep 1234
```

### No IQ Data
```bash
# Test EvilCrow directly
screen /dev/cu.usbserial-11440 115200
# Type: rx_start
# Should see binary data
```

## 📊 Bridge Features

### Supported RTL-TCP Commands
- ✅ Set Frequency
- ✅ Set Sample Rate  
- ✅ Set Gain
- ✅ Set Gain Mode
- ✅ Set AGC Mode
- ✅ Set Bias Tee
- ✅ IQ Data Streaming

### EvilCrow Commands Used
```bash
board_id_read          # Get device info
set_freq <hz>          # Set center frequency
set_sample_rate <hz>   # Set sample rate
set_gain <db>          # Set RF gain
set_gain_mode <mode>   # Set gain mode (auto/manual)
set_agc <on/off>       # Set AGC
set_bias_tee <on/off>  # Set bias tee
rx_start               # Start receiving
rx_stop                # Stop receiving
```

## 🎛️ URH Usage Tips

### Recording Signals
1. **Set appropriate frequency** (e.g., 433.92 MHz for ISM band)
2. **Adjust sample rate** (higher = more bandwidth, more CPU)
3. **Set gain** (start with 20 dB, adjust as needed)
4. **Click Start** to begin recording
5. **Transmit your signal** (key fob, remote, etc.)
6. **Click Stop** when done

### Analyzing Signals
1. **URH will show the captured waveform**
2. **Use zoom tools** to examine signal details
3. **URH can auto-detect** modulation and protocols
4. **Export data** for further analysis

### Signal Generation
1. **URH can replay captured signals**
2. **Modify signals** before transmission
3. **Generate custom waveforms**

## 🚀 Advanced Usage

### Custom Frequencies
```bash
# Start bridge with different frequency
python3 evilcrow_urh_bridge.py -f 315000000  # 315 MHz
python3 evilcrow_urh_bridge.py -f 868000000  # 868 MHz
python3 evilcrow_urh_bridge.py -f 915000000  # 915 MHz
```

### Higher Sample Rates
```bash
# Use higher sample rate for wider bandwidth
python3 evilcrow_urh_bridge.py -s 4000000    # 4 MHz
python3 evilcrow_urh_bridge.py -s 8000000    # 8 MHz
```

### Multiple Connections
```bash
# Use different TCP ports for multiple instances
python3 evilcrow_urh_bridge.py -t 1234       # First instance
python3 evilcrow_urh_bridge.py -t 1235       # Second instance
```

## 📡 Frequency Bands

### Common ISM Bands
- **315 MHz** - North American garage doors, car keys
- **433.92 MHz** - European ISM band, weather stations
- **868 MHz** - European ISM band, LoRa, smart meters  
- **915 MHz** - North American ISM band, LoRa, ZigBee
- **2.4 GHz** - WiFi, Bluetooth, ZigBee

### Example URH Settings
```
433 MHz Band:
- Frequency: 433920000 Hz
- Sample Rate: 2000000 Hz (covers ±1 MHz)
- Gain: 20 dB

315 MHz Band:
- Frequency: 315000000 Hz  
- Sample Rate: 2000000 Hz
- Gain: 25 dB

868 MHz Band:
- Frequency: 868000000 Hz
- Sample Rate: 2000000 Hz  
- Gain: 15 dB
```

## 🎯 Success Indicators

### Bridge Running Correctly
```
[15:14:23.468] 🚀 EvilCrow RF v2 - URH Bridge Starting...
[15:14:23.468] 🔗 Connecting to EvilCrow on /dev/cu.usbserial-11440...
[15:14:23.468] 📡 EvilCrow response: HACKRF_ONE
[15:14:23.468] ⚙️  Initializing EvilCrow SDR...
[15:14:23.468] ✅ EvilCrow SDR initialized successfully
[15:14:23.468] 🌐 RTL-TCP server listening on 127.0.0.1:1234
[15:14:23.468] 📱 Ready for URH connection!
```

### URH Connected
```
[15:14:25.123] 🔗 URH connected from ('127.0.0.1', 54321)
[15:14:25.124] 🚀 Starting IQ data streaming...
[15:14:25.125] 📊 Streamed 100000 samples
```

### Signal Detected in URH
- **Waveform appears** in URH display
- **Signal strength indicator** shows activity
- **Frequency spectrum** shows peaks
- **Protocol analysis** may auto-detect signal type

## 🆘 Getting Help

If you encounter issues:

1. **Run the test script** first: `python3 test_evilcrow.py`
2. **Check EvilCrow firmware** is the SDR version
3. **Verify serial port** is correct
4. **Ensure no other software** is using the port
5. **Check URH logs** for error messages
6. **Try different frequencies/sample rates**

The bridge provides detailed logging to help diagnose connection and streaming issues.
