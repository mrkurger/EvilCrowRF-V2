#!/usr/bin/env python3
"""
Simple RTL-TCP Test Server
Minimal implementation to test URH connection protocol
"""

import socket
import threading
import time
import struct
import random

class SimpleRTLTCPServer:
    def __init__(self, port=1234):
        self.port = port
        self.running = False
        self.client_socket = None

    def log(self, message):
        print(f"[{time.strftime('%H:%M:%S')}] {message}")

    def handle_rtl_command(self, data):
        """Handle RTL-TCP commands"""
        if len(data) < 5:
            return

        cmd = data[0]
        param = struct.unpack('>I', data[1:5])[0]

        if cmd == 0x01:  # Set frequency
            self.log(f"📻 Set frequency: {param} Hz")
        elif cmd == 0x02:  # Set sample rate
            self.log(f"📊 Set sample rate: {param} Hz")
        elif cmd == 0x04:  # Set gain
            gain = param / 10.0
            self.log(f"📈 Set gain: {gain} dB")
        else:
            self.log(f"❓ Unknown command: 0x{cmd:02x} param: {param}")

    def stream_data(self):
        """Stream IQ data"""
        self.log("🚀 Starting data stream...")

        sample_count = 0

        try:
            while self.running and self.client_socket:
                # Generate 1024 IQ samples (2048 bytes)
                buffer = bytearray()
                for _ in range(1024):
                    # Generate noise around 127
                    i_val = random.randint(120, 135)
                    q_val = random.randint(120, 135)
                    buffer.extend([i_val, q_val])

                # Send buffer
                try:
                    self.client_socket.sendall(buffer)
                    sample_count += 1024

                    if sample_count % 100000 == 0:
                        self.log(f"📊 Sent {sample_count//1000}k samples")

                    # Control rate
                    time.sleep(0.0005)

                except Exception as e:
                    self.log(f"❌ Stream error: {e}")
                    break

        except Exception as e:
            self.log(f"❌ Streaming error: {e}")
        finally:
            self.log("⏹️  Streaming stopped")

    def handle_client(self, client_socket, addr):
        """Handle client connection"""
        self.client_socket = client_socket
        self.log(f"🔗 Client connected from {addr}")

        try:
            # Send RTL-TCP DongleInfo header (12 bytes total)
            # Magic: "RTL0" (4 bytes)
            # Tuner Type: uint32 (4 bytes)
            # Tuner Gain Type: uint32 (4 bytes)
            magic = b"RTL0"  # 4 bytes
            tuner_type = struct.pack('>I', 1)  # 4 bytes - Generic RTL2832U
            tuner_gain_type = struct.pack('>I', 1)  # 4 bytes - Manual gain control

            dongle_info = magic + tuner_type + tuner_gain_type
            client_socket.sendall(dongle_info)
            self.log("📡 Sent RTL-TCP DongleInfo header (12 bytes)")

            # Start streaming
            self.running = True
            stream_thread = threading.Thread(target=self.stream_data)
            stream_thread.daemon = True
            stream_thread.start()

            # Handle commands
            while self.running:
                try:
                    data = client_socket.recv(1024)
                    if not data:
                        self.log("📡 Client disconnected")
                        break

                    self.log(f"📨 Received {len(data)} bytes")

                    # Process commands
                    for i in range(0, len(data), 5):
                        if i + 4 < len(data):
                            self.handle_rtl_command(data[i:i+5])

                except Exception as e:
                    self.log(f"❌ Command error: {e}")
                    break

        except Exception as e:
            self.log(f"❌ Client error: {e}")
        finally:
            self.running = False
            try:
                client_socket.close()
            except:
                pass
            self.client_socket = None
            self.log("🔌 Client disconnected")

    def run(self):
        """Run server"""
        self.log("🚀 Simple RTL-TCP Test Server Starting...")

        try:
            server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server_socket.bind(('127.0.0.1', self.port))
            server_socket.listen(1)

            self.log(f"🌐 Listening on 127.0.0.1:{self.port}")
            self.log("📱 Ready for URH connection!")

            while True:
                try:
                    client_socket, addr = server_socket.accept()
                    self.handle_client(client_socket, addr)
                except KeyboardInterrupt:
                    break
                except Exception as e:
                    self.log(f"❌ Accept error: {e}")
                    time.sleep(1)

        except Exception as e:
            self.log(f"❌ Server error: {e}")
        finally:
            try:
                server_socket.close()
            except:
                pass
            self.log("🧹 Server stopped")

def main():
    server = SimpleRTLTCPServer()
    server.run()

if __name__ == "__main__":
    main()
