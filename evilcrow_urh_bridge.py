#!/usr/bin/env python3
"""
EvilCrow RF v2 - URH Bridge
Makes EvilCrow compatible with Universal Radio Hacker (URH)
Emulates RTL-SDR TCP protocol for seamless integration
"""

import socket
import serial
import threading
import struct
import time
import sys
import signal
import argparse
from datetime import datetime

class EvilCrowURHBridge:
    def __init__(self, serial_port="/dev/cu.usbserial-11440", tcp_port=1234, sample_rate=2000000, frequency=433920000):
        self.serial_port = serial_port
        self.tcp_port = tcp_port
        self.sample_rate = sample_rate
        self.frequency = frequency
        self.ser = None
        self.server_socket = None
        self.client_socket = None
        self.running = False
        self.streaming = False

        # RTL-TCP command constants
        self.RTL_TCP_SET_FREQ = 0x01
        self.RTL_TCP_SET_SAMPLE_RATE = 0x02
        self.RTL_TCP_SET_GAIN_MODE = 0x03
        self.RTL_TCP_SET_GAIN = 0x04
        self.RTL_TCP_SET_FREQ_CORRECTION = 0x05
        self.RTL_TCP_SET_IF_STAGE = 0x06
        self.RTL_TCP_SET_TEST_MODE = 0x07
        self.RTL_TCP_SET_AGC_MODE = 0x08
        self.RTL_TCP_SET_DIRECT_SAMPLING = 0x09
        self.RTL_TCP_SET_OFFSET_TUNING = 0x0a
        self.RTL_TCP_SET_RTL_CRYSTAL = 0x0b
        self.RTL_TCP_SET_TUNER_CRYSTAL = 0x0c
        self.RTL_TCP_SET_TUNER_GAIN_BY_INDEX = 0x0d
        self.RTL_TCP_SET_BIAS_TEE = 0x0e

    def log(self, message):
        """Log with timestamp"""
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        print(f"[{timestamp}] {message}")

    def connect_evilcrow(self):
        """Connect to EvilCrow SDR"""
        try:
            self.log(f"🔗 Connecting to EvilCrow on {self.serial_port}...")
            self.ser = serial.Serial(self.serial_port, 115200, timeout=2)
            time.sleep(2)  # Wait for connection

            # Clear any pending data first
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()

            # Test connection with simple command
            self.log("📡 Testing EvilCrow connection...")
            self.ser.write(b"status\n")
            time.sleep(0.5)

            # Read response, handling potential binary data
            response = ""
            attempts = 0
            while attempts < 10:  # Try up to 10 times
                if self.ser.in_waiting > 0:
                    try:
                        data = self.ser.read(self.ser.in_waiting)
                        response += data.decode('utf-8', errors='ignore')
                        break
                    except Exception:
                        pass
                time.sleep(0.1)
                attempts += 1

            if "EvilCrow" in response or "HACKRF" in response:
                self.log(f"📡 EvilCrow connected successfully!")
            else:
                self.log(f"📡 Response: {response[:100]}...")  # Show first 100 chars

            # Initialize SDR parameters
            self.log("⚙️  Initializing EvilCrow SDR...")
            self.ser.write(f"set_freq {self.frequency}\n".encode())
            time.sleep(0.2)
            self.ser.write(f"set_sample_rate {self.sample_rate}\n".encode())
            time.sleep(0.2)
            self.ser.write(b"set_gain 20\n")
            time.sleep(0.2)

            # Clear any pending data
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()

            self.log("✅ EvilCrow SDR initialized successfully")
            return True

        except Exception as e:
            self.log(f"❌ Failed to connect to EvilCrow: {e}")
            if self.ser:
                try:
                    self.ser.close()
                except:
                    pass
            return False

    def start_tcp_server(self):
        """Start RTL-TCP compatible server"""
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind(('127.0.0.1', self.tcp_port))
            self.server_socket.listen(1)

            self.log(f"🌐 RTL-TCP server listening on 127.0.0.1:{self.tcp_port}")
            self.log("📱 Ready for URH connection!")
            self.log("💡 In URH: Select RTL-TCP device, IP: 127.0.0.1, Port: 1234")

            return True

        except Exception as e:
            self.log(f"❌ Failed to start TCP server: {e}")
            return False

    def handle_rtl_tcp_command(self, data):
        """Handle RTL-TCP protocol commands from URH"""
        if len(data) < 5:
            return

        cmd = data[0]
        param = struct.unpack('>I', data[1:5])[0]  # Big-endian 32-bit

        if cmd == self.RTL_TCP_SET_FREQ:
            self.frequency = param
            self.log(f"📻 Setting frequency: {param} Hz")
            self.ser.write(f"set_freq {param}\n".encode())

        elif cmd == self.RTL_TCP_SET_SAMPLE_RATE:
            self.sample_rate = param
            self.log(f"📊 Setting sample rate: {param} Hz")
            self.ser.write(f"set_sample_rate {param}\n".encode())

        elif cmd == self.RTL_TCP_SET_GAIN:
            gain = param / 10.0  # RTL-TCP uses tenths of dB
            self.log(f"📈 Setting gain: {gain} dB")
            self.ser.write(f"set_gain {int(gain)}\n".encode())

        elif cmd == self.RTL_TCP_SET_GAIN_MODE:
            mode = "manual" if param == 1 else "auto"
            self.log(f"🎛️  Setting gain mode: {mode}")
            self.ser.write(f"set_gain_mode {mode}\n".encode())

        elif cmd == self.RTL_TCP_SET_AGC_MODE:
            agc = "on" if param == 1 else "off"
            self.log(f"🔄 Setting AGC: {agc}")
            self.ser.write(f"set_agc {agc}\n".encode())

        elif cmd == self.RTL_TCP_SET_BIAS_TEE:
            bias = "on" if param == 1 else "off"
            self.log(f"⚡ Setting bias tee: {bias}")
            self.ser.write(f"set_bias_tee {bias}\n".encode())

        else:
            self.log(f"❓ Unknown RTL-TCP command: 0x{cmd:02x} param: {param}")

    def stream_iq_data(self):
        """Stream IQ data from EvilCrow to URH"""
        self.log("🚀 Starting IQ data streaming...")

        # Start RX mode on EvilCrow
        self.ser.write(b"rx_start\n")
        time.sleep(0.5)  # Give more time for RX to start

        buffer = bytearray()
        samples_sent = 0
        last_log_time = time.time()

        try:
            while self.streaming and self.client_socket:
                # Read data from EvilCrow
                if self.ser.in_waiting > 0:
                    try:
                        data = self.ser.read(self.ser.in_waiting)
                        # Filter out text responses, only keep binary data
                        for byte in data:
                            # Only add bytes that look like IQ data (not ASCII text)
                            if byte < 32 or byte > 126:  # Non-printable = likely binary
                                buffer.append(byte)
                    except Exception as e:
                        self.log(f"⚠️  Read error: {e}")
                        continue

                # Process complete IQ samples (4 bytes each: I16 + Q16)
                while len(buffer) >= 4:
                    try:
                        # Extract I and Q values (16-bit signed, little-endian)
                        i_raw = struct.unpack('<h', buffer[0:2])[0]
                        q_raw = struct.unpack('<h', buffer[2:4])[0]
                        buffer = buffer[4:]

                        # Convert to 8-bit unsigned (RTL-SDR format)
                        # Scale from [-32768, 32767] to [0, 255]
                        i_8bit = int((i_raw + 32768) / 256) & 0xFF
                        q_8bit = int((q_raw + 32768) / 256) & 0xFF

                        # Send to URH
                        self.client_socket.send(bytes([i_8bit, q_8bit]))
                        samples_sent += 1

                        # Log progress every 10 seconds
                        current_time = time.time()
                        if current_time - last_log_time > 10:
                            self.log(f"📊 Streamed {samples_sent} samples ({samples_sent/10:.0f} samples/sec)")
                            last_log_time = current_time

                    except (BrokenPipeError, ConnectionResetError):
                        self.log("🔌 URH disconnected")
                        return
                    except struct.error:
                        # Skip malformed data
                        buffer = buffer[1:]  # Skip one byte and try again
                        continue
                    except Exception as e:
                        self.log(f"⚠️  Sample processing error: {e}")
                        continue

                # If no data available, send dummy samples to keep connection alive
                if self.ser.in_waiting == 0:
                    try:
                        # Send noise-like dummy data
                        import random
                        dummy_i = random.randint(120, 135)  # Around center
                        dummy_q = random.randint(120, 135)
                        self.client_socket.send(bytes([dummy_i, dummy_q]))
                        samples_sent += 1
                    except:
                        break

                time.sleep(0.001)  # Small delay

        except Exception as e:
            self.log(f"❌ Streaming error: {e}")
        finally:
            # Stop RX mode
            if self.ser:
                try:
                    self.ser.write(b"rx_stop\n")
                except:
                    pass
            self.streaming = False
            self.log(f"⏹️  IQ streaming stopped (sent {samples_sent} samples)")

    def handle_client(self, client_socket, addr):
        """Handle URH client connection"""
        self.client_socket = client_socket
        self.log(f"🔗 URH connected from {addr}")

        # Send RTL-TCP magic bytes (optional)
        magic = b"RTL0"
        try:
            client_socket.send(magic)
        except:
            self.log("❌ Failed to send magic bytes")
            return

        # Start streaming thread
        self.streaming = True
        stream_thread = threading.Thread(target=self.stream_iq_data)
        stream_thread.daemon = True
        stream_thread.start()

        # Send some initial dummy data to establish connection
        try:
            for _ in range(1000):  # Send 1000 initial samples
                dummy_data = bytes([127, 127])  # Center values
                client_socket.send(dummy_data)
            self.log("📡 Sent initial data to URH")
        except Exception as e:
            self.log(f"❌ Failed to send initial data: {e}")
            self.streaming = False
            return

        # Handle commands from URH
        try:
            while self.running and self.streaming:
                client_socket.settimeout(1.0)  # 1 second timeout
                try:
                    data = client_socket.recv(1024)
                    if not data:
                        break

                    # Process RTL-TCP commands
                    i = 0
                    while i + 4 < len(data):
                        self.handle_rtl_tcp_command(data[i:i+5])
                        i += 5
                except Exception:  # Changed from socket.timeout to generic Exception
                    # Timeout is normal, just continue
                    continue

        except Exception as e:
            self.log(f"❌ Client handling error: {e}")
        finally:
            self.streaming = False
            try:
                client_socket.close()
            except:
                pass
            self.client_socket = None
            self.log("🔌 URH client disconnected")

    def run(self):
        """Main bridge loop"""
        self.log("🚀 EvilCrow RF v2 - URH Bridge Starting...")

        # Connect to EvilCrow
        if not self.connect_evilcrow():
            return False

        # Start TCP server
        if not self.start_tcp_server():
            return False

        self.running = True

        try:
            while self.running:
                # Accept URH connections
                client_socket, addr = self.server_socket.accept()

                # Handle client in separate thread
                client_thread = threading.Thread(
                    target=self.handle_client,
                    args=(client_socket, addr)
                )
                client_thread.daemon = True
                client_thread.start()

        except KeyboardInterrupt:
            self.log("⏹️  Shutting down bridge...")
        except Exception as e:
            self.log(f"❌ Server error: {e}")
        finally:
            self.cleanup()

        return True

    def cleanup(self):
        """Clean up resources"""
        self.running = False
        self.streaming = False

        if self.client_socket:
            self.client_socket.close()

        if self.server_socket:
            self.server_socket.close()

        if self.ser:
            self.ser.write(b"rx_stop\n")
            self.ser.close()

        self.log("🧹 Cleanup completed")

def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\n⏹️  Shutting down bridge...")
    sys.exit(0)

def main():
    parser = argparse.ArgumentParser(description='EvilCrow RF v2 - URH Bridge')
    parser.add_argument('-p', '--port', default='/dev/cu.usbserial-11440',
                       help='Serial port (default: /dev/cu.usbserial-11440)')
    parser.add_argument('-t', '--tcp-port', type=int, default=1234,
                       help='TCP port for RTL-TCP server (default: 1234)')
    parser.add_argument('-f', '--frequency', type=int, default=433920000,
                       help='Initial frequency in Hz (default: 433920000)')
    parser.add_argument('-s', '--sample-rate', type=int, default=2000000,
                       help='Sample rate in Hz (default: 2000000)')

    args = parser.parse_args()

    # Set up signal handler
    signal.signal(signal.SIGINT, signal_handler)

    # Create and run bridge
    bridge = EvilCrowURHBridge(
        serial_port=args.port,
        tcp_port=args.tcp_port,
        sample_rate=args.sample_rate,
        frequency=args.frequency
    )

    success = bridge.run()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
