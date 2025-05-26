#!/usr/bin/env python3
"""
Simple EvilCrow RF v2 - URH Bridge
Minimal implementation to get URH working with EvilCrow
"""

import socket
import serial
import threading
import time
import sys
import struct

class SimpleURHBridge:
    def __init__(self, serial_port="/dev/cu.usbserial-140", tcp_port=1234):
        self.serial_port = serial_port
        self.tcp_port = tcp_port
        self.ser = None
        self.server_socket = None
        self.running = False

    def log(self, message):
        print(f"[{time.strftime('%H:%M:%S')}] {message}")

    def connect_evilcrow(self):
        """Connect to EvilCrow"""
        try:
            self.log(f"🔗 Connecting to EvilCrow on {self.serial_port}...")
            self.ser = serial.Serial(self.serial_port, 115200, timeout=1)
            time.sleep(2)

            # Clear buffers
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()

            # Test connection
            self.ser.write(b"status\n")
            time.sleep(0.5)

            response = ""
            if self.ser.in_waiting > 0:
                try:
                    response = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                except:
                    pass

            self.log("✅ EvilCrow connected")

            # Initialize
            self.ser.write(b"set_freq 433920000\n")
            time.sleep(0.2)
            self.ser.write(b"set_sample_rate 2000000\n")
            time.sleep(0.2)
            self.ser.write(b"set_gain 20\n")
            time.sleep(0.2)

            return True

        except Exception as e:
            self.log(f"❌ Failed to connect: {e}")
            return False

    def start_tcp_server(self):
        """Start TCP server"""
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind(('127.0.0.1', self.tcp_port))
            self.server_socket.listen(1)

            self.log(f"🌐 TCP server listening on 127.0.0.1:{self.tcp_port}")
            self.log("📱 Ready for URH connection!")

            return True

        except Exception as e:
            self.log(f"❌ Failed to start server: {e}")
            return False

    def handle_rtl_command(self, data):
        """Handle RTL-TCP commands from URH"""
        if len(data) < 5:
            return

        cmd = data[0]
        param = struct.unpack('>I', data[1:5])[0]  # Big-endian 32-bit

        if cmd == 0x01:  # Set frequency
            self.log(f"📻 URH set frequency: {param} Hz")
            self.ser.write(f"set_freq {param}\n".encode())
        elif cmd == 0x02:  # Set sample rate
            self.log(f"📊 URH set sample rate: {param} Hz")
            self.ser.write(f"set_sample_rate {param}\n".encode())
        elif cmd == 0x04:  # Set gain
            gain = param / 10.0  # RTL-TCP uses tenths of dB
            self.log(f"📈 URH set gain: {gain} dB")
            self.ser.write(f"set_gain {int(gain)}\n".encode())
        else:
            self.log(f"❓ Unknown RTL command: 0x{cmd:02x} param: {param}")

    def handle_client(self, client_socket, addr):
        """Handle URH client"""
        self.log(f"🔗 URH connected from {addr}")

        try:
            # Set socket options for better reliability
            client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
            client_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

            # Send RTL-TCP magic bytes (4 bytes: "RTL0")
            magic = b"RTL0"
            client_socket.sendall(magic)
            self.log("📡 Sent RTL-TCP magic bytes")

            # Start RX on EvilCrow
            self.ser.write(b"rx_start\n")
            time.sleep(0.5)
            self.log("🚀 Started EvilCrow RX mode")

            # Set socket to non-blocking for command handling
            client_socket.settimeout(0.001)  # 1ms timeout

            # Send continuous IQ data stream
            sample_count = 0

            # Pre-generate a chunk of dummy data for efficiency
            import random
            dummy_chunk = bytearray()
            for _ in range(4096):  # 2048 IQ samples (smaller chunks)
                i_val = random.randint(110, 145)  # Wider range for more realistic noise
                q_val = random.randint(110, 145)
                dummy_chunk.extend([i_val, q_val])

            self.log("📊 Starting continuous IQ data stream...")

            while True:
                # Check for commands from URH
                try:
                    cmd_data = client_socket.recv(1024)
                    if cmd_data:
                        self.handle_rtl_command(cmd_data)
                except socket.timeout:
                    pass  # No command received, continue
                except socket.error:
                    break  # Connection lost

                # Send the pre-generated chunk
                try:
                    client_socket.sendall(dummy_chunk)
                    sample_count += len(dummy_chunk) // 2  # Each sample is 2 bytes (I+Q)

                    # Log progress every 1M samples
                    if sample_count % 1000000 == 0:
                        self.log(f"📊 Streamed {sample_count//1000000}M samples")

                    # Small delay to control data rate (~2MHz sample rate)
                    time.sleep(0.002)  # 2ms delay for ~4096 samples = ~2MHz

                except socket.error as e:
                    self.log(f"🔌 Socket error: {e}")
                    break
                except Exception as e:
                    self.log(f"❌ Send error: {e}")
                    break

        except Exception as e:
            self.log(f"❌ Client error: {e}")
        finally:
            try:
                self.ser.write(b"rx_stop\n")
                self.log("⏹️  Stopped EvilCrow RX mode")
            except:
                pass
            try:
                client_socket.close()
            except:
                pass
            self.log("🔌 URH disconnected")

    def run(self):
        """Main loop"""
        self.log("🚀 Simple URH Bridge Starting...")

        if not self.connect_evilcrow():
            return False

        if not self.start_tcp_server():
            return False

        self.running = True

        try:
            while self.running:
                client_socket, addr = self.server_socket.accept()

                # Handle each client in a thread
                client_thread = threading.Thread(
                    target=self.handle_client,
                    args=(client_socket, addr)
                )
                client_thread.daemon = True
                client_thread.start()

        except KeyboardInterrupt:
            self.log("⏹️  Shutting down...")
        except Exception as e:
            self.log(f"❌ Server error: {e}")
        finally:
            self.cleanup()

        return True

    def cleanup(self):
        """Cleanup"""
        self.running = False

        if self.server_socket:
            self.server_socket.close()

        if self.ser:
            try:
                self.ser.write(b"rx_stop\n")
                self.ser.close()
            except:
                pass

        self.log("🧹 Cleanup completed")

def main():
    # Check for port argument
    port = "/dev/cu.usbserial-140"
    if len(sys.argv) > 1:
        port = sys.argv[1]

    bridge = SimpleURHBridge(serial_port=port)
    bridge.run()

if __name__ == "__main__":
    main()
