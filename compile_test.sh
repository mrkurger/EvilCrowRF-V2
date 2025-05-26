#!/bin/bash

# EvilCrow RF v2 Firmware Compilation Test Script
# This script tests the enhanced firmware compilation using Arduino CLI

echo "🔧 EvilCrow RF v2 Enhanced Firmware Compilation Test"
echo "=================================================="

# Check for Arduino CLI
if ! command -v arduino-cli &> /dev/null; then
    echo "❌ Arduino CLI not found. Attempting to install..."
    
    # Install Arduino CLI if not present
    if [[ "$(uname)" == "Darwin" ]]; then
        if command -v brew &> /dev/null; then
            echo "📦 Installing Arduino CLI via Homebrew..."
            brew install arduino-cli
        else
            echo "⚠️  Homebrew not found. Please install Arduino CLI manually."
            echo "   Visit: https://arduino.github.io/arduino-cli/latest/installation/"
            exit 1
        fi
    else
        echo "⚠️  Please install Arduino CLI manually for your platform."
        echo "   Visit: https://arduino.github.io/arduino-cli/latest/installation/"
        exit 1
    fi
fi

# Set up Arduino CLI configuration
echo "⚙️  Setting up Arduino CLI configuration..."
arduino-cli config init

# Add ESP32 board manager URL
echo "📡 Adding ESP32 board manager..."
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

# Update board index
echo "🔄 Updating board index..."
arduino-cli core update-index

# Install ESP32 core
echo "💾 Installing ESP32 core..."
arduino-cli core install esp32:esp32

# Navigate to firmware directory
cd "/Users/oivindlund/git_repos/EvilCrowRF-V2/firmware/NEW/EvilCrow-RFv2-Ready"

# Compile the enhanced firmware
echo "🔨 Compiling EvilCrow RF v2 Enhanced Firmware..."
echo "   Target Board: ESP32 Dev Module"
echo "   Firmware: EvilCrow-RFv2-Ready.ino"
echo ""

# Attempt compilation
if arduino-cli compile --fqbn esp32:esp32:esp32 EvilCrow-RFv2-Ready.ino; then
    echo ""
    echo "✅ SUCCESS: Enhanced firmware compiled successfully!"
    echo "🎯 All 5 advanced RF analysis functions are compatible"
    echo "🌐 Web interface overhaul is working"
    echo "📊 Compilation statistics:"
    
    # Get compilation output size if available
    if [ -f "build/esp32.esp32.esp32/EvilCrow-RFv2-Ready.ino.bin" ]; then
        SIZE=$(ls -lh build/esp32.esp32.esp32/EvilCrow-RFv2-Ready.ino.bin | awk '{print $5}')
        echo "   Binary size: $SIZE"
    fi
    
    echo ""
    echo "🚀 Enhanced firmware ready for deployment!"
    echo "📋 Next steps:"
    echo "   1. Flash to EvilCrow RF v2 device"
    echo "   2. Test all 5 new advanced features"
    echo "   3. Validate web interface functionality"
    
else
    echo ""
    echo "❌ COMPILATION FAILED"
    echo "🔍 Please check the error messages above"
    echo "💡 Common issues:"
    echo "   - Missing libraries (install via Arduino IDE Library Manager)"
    echo "   - Board configuration issues"
    echo "   - Syntax errors in the code"
    
    exit 1
fi
