#!/usr/bin/env python3
"""
EvilCrow RF v2 - Connection Test
Test script to verify EvilCrow SDR communication
"""

import serial
import time
import sys

def test_evilcrow(port="/dev/cu.usbserial-140"):
    """Test EvilCrow SDR connection and commands"""

    print("🚀 EvilCrow RF v2 - Connection Test")
    print("=" * 50)

    try:
        print(f"🔗 Connecting to {port}...")
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(1)

        # Test commands
        commands = [
            ("board_id_read", "Get board ID"),
            ("status", "Get status"),
            ("get_freq", "Get frequency"),
            ("get_sample_rate", "Get sample rate"),
            ("set_freq 433920000", "Set frequency to 433.92 MHz"),
            ("set_sample_rate 2000000", "Set sample rate to 2 MHz"),
            ("set_gain 20", "Set gain to 20 dB"),
            ("get_gain", "Get current gain"),
        ]

        print("📡 Testing EvilCrow commands...")
        print("-" * 50)

        for cmd, description in commands:
            print(f"📤 {description}: {cmd}")

            # Clear input buffer
            ser.reset_input_buffer()

            # Send command
            ser.write(f"{cmd}\n".encode())
            time.sleep(0.3)

            # Read response
            response = ""
            while ser.in_waiting > 0:
                try:
                    response += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                except UnicodeDecodeError:
                    response += "[Binary data received]"
                time.sleep(0.1)

            if response.strip():
                print(f"📥 Response: {response.strip()}")
            else:
                print("📥 No response")

            print()

        # Test IQ data streaming
        print("🌊 Testing IQ data streaming...")
        ser.write(b"rx_start\n")
        time.sleep(0.5)

        samples_received = 0
        start_time = time.time()

        while time.time() - start_time < 3:  # Test for 3 seconds
            if ser.in_waiting >= 4:
                data = ser.read(4)
                if len(data) == 4:
                    samples_received += 1

        ser.write(b"rx_stop\n")

        print(f"📊 Received {samples_received} IQ samples in 3 seconds")
        print(f"📈 Sample rate: ~{samples_received/3:.0f} samples/sec")

        ser.close()
        print("✅ EvilCrow test completed successfully!")
        return True

    except serial.SerialException as e:
        print(f"❌ Serial connection error: {e}")
        print("💡 Make sure EvilCrow is connected and the port is correct")
        return False

    except Exception as e:
        print(f"❌ Test failed: {e}")
        return False

def main():
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        port = "/dev/cu.usbserial-140"

    success = test_evilcrow(port)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
