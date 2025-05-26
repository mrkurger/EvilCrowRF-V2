#!/usr/bin/env python3
"""
RTL-TCP Connection Test
Test tool to diagnose RTL-TCP connection issues
"""

import socket
import time
import sys

def test_rtl_tcp_connection(host="127.0.0.1", port=1234):
    """Test RTL-TCP connection like URH would"""
    
    print("🔍 RTL-TCP Connection Test")
    print("=" * 50)
    
    try:
        print(f"🔗 Connecting to {host}:{port}...")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)  # 5 second timeout
        
        # Connect
        sock.connect((host, port))
        print("✅ TCP connection established")
        
        # Read magic bytes
        magic = sock.recv(4)
        print(f"📡 Received magic bytes: {magic}")
        
        if magic == b"RTL0":
            print("✅ Correct RTL-TCP magic bytes received")
        else:
            print(f"❌ Unexpected magic bytes: {magic}")
        
        # Send a frequency command (like URH would)
        freq_cmd = bytes([0x01]) + (433920000).to_bytes(4, 'big')
        sock.send(freq_cmd)
        print("📻 Sent frequency command (433.92 MHz)")
        
        # Try to receive some IQ data
        print("📊 Receiving IQ data...")
        data_received = 0
        start_time = time.time()
        
        while time.time() - start_time < 5:  # Test for 5 seconds
            try:
                data = sock.recv(8192)
                if data:
                    data_received += len(data)
                    if data_received % 100000 == 0:
                        print(f"📈 Received {data_received} bytes")
                else:
                    print("❌ No data received")
                    break
            except socket.timeout:
                print("⏰ Timeout waiting for data")
                break
            except Exception as e:
                print(f"❌ Error receiving data: {e}")
                break
        
        print(f"📊 Total data received: {data_received} bytes")
        print(f"📈 Sample rate: ~{data_received/5/2:.0f} samples/sec")
        
        if data_received > 0:
            print("✅ RTL-TCP connection working correctly!")
        else:
            print("❌ No data received - connection issue")
        
        sock.close()
        return data_received > 0
        
    except ConnectionRefused:
        print("❌ Connection refused - is the bridge running?")
        return False
    except socket.timeout:
        print("❌ Connection timeout")
        return False
    except Exception as e:
        print(f"❌ Connection error: {e}")
        return False

def test_urh_compatibility():
    """Test URH-specific connection behavior"""
    
    print("\n🎯 URH Compatibility Test")
    print("=" * 50)
    
    try:
        # Simulate URH's connection pattern
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        
        print("🔗 Connecting like URH...")
        sock.connect(("127.0.0.1", 1234))
        
        # URH expects immediate data
        print("📡 Waiting for immediate data...")
        data = sock.recv(4)  # Magic bytes
        print(f"📦 First 4 bytes: {data}")
        
        # URH sends commands quickly
        commands = [
            (0x01, 433920000),  # Set frequency
            (0x02, 2000000),    # Set sample rate
            (0x04, 200),        # Set gain (20.0 dB)
        ]
        
        for cmd, param in commands:
            cmd_bytes = bytes([cmd]) + param.to_bytes(4, 'big')
            sock.send(cmd_bytes)
            print(f"📤 Sent command: 0x{cmd:02x} param: {param}")
            time.sleep(0.1)
        
        # Check if we can receive continuous data
        print("📊 Testing continuous data stream...")
        total_received = 0
        
        for i in range(10):  # 10 iterations
            try:
                data = sock.recv(4096)
                total_received += len(data)
                print(f"📈 Iteration {i+1}: {len(data)} bytes")
                time.sleep(0.1)
            except Exception as e:
                print(f"❌ Error in iteration {i+1}: {e}")
                break
        
        print(f"📊 Total received: {total_received} bytes")
        
        sock.close()
        
        if total_received > 10000:  # Should receive plenty of data
            print("✅ URH compatibility test passed!")
            return True
        else:
            print("❌ URH compatibility test failed - insufficient data")
            return False
            
    except Exception as e:
        print(f"❌ URH compatibility test error: {e}")
        return False

def main():
    print("🚀 RTL-TCP Bridge Diagnostic Tool")
    print("=" * 60)
    
    # Test basic connection
    basic_test = test_rtl_tcp_connection()
    
    # Test URH compatibility
    urh_test = test_urh_compatibility()
    
    print("\n📋 Test Summary")
    print("=" * 30)
    print(f"Basic RTL-TCP: {'✅ PASS' if basic_test else '❌ FAIL'}")
    print(f"URH Compatible: {'✅ PASS' if urh_test else '❌ FAIL'}")
    
    if basic_test and urh_test:
        print("\n🎉 All tests passed! URH should work correctly.")
    else:
        print("\n⚠️  Some tests failed. Check bridge implementation.")
    
    return basic_test and urh_test

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
