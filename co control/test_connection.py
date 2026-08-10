# test_connection.py
# Simple connectivity test for STM32 W5100 Ethernet
# Tests: ping, TCP connect to port 5000, TCP connect to port 5001

import socket
import subprocess
import sys
import time

STM32_IP = "10.87.243.100"
CTRL_PORT = 5000
VIDEO_PORT = 5001

def test_ping():
    """Test ICMP ping to the STM32."""
    print(f"\n[1/3] Testing ping to {STM32_IP}...")
    try:
        result = subprocess.run(
            ["ping", "-n", "2", "-w", "2000", STM32_IP],
            capture_output=True, text=True, timeout=10
        )
        if "Reply from" in result.stdout:
            print("  PING: SUCCESS - STM32 is reachable!")
            return True
        elif "Destination host unreachable" in result.stdout:
            print("  PING: FAIL - Destination host unreachable (no ARP response)")
            print("  -> STM32 not on this network. Check cable/port.")
        elif "Request timed out" in result.stdout:
            print("  PING: FAIL - Request timed out (no response)")
            print("  -> STM32 may be on network but not responding to ping.")
        else:
            print(f"  PING: FAIL - {result.stdout.strip()}")
        return False
    except Exception as e:
        print(f"  PING: ERROR - {e}")
        return False

def test_tcp(port, name):
    """Test TCP connection to a specific port."""
    print(f"\n[{'2' if port == CTRL_PORT else '3'}/3] Testing TCP connect to {STM32_IP}:{port} ({name})...")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        s.connect((STM32_IP, port))
        print(f"  TCP {name} - SUCCESS! Connected to port {port}")
        s.close()
        return True
    except socket.timeout:
        print(f"  TCP {name} - FAIL - Connection timed out (no response)")
        print("  -> STM32 not listening or not reachable.")
    except ConnectionRefusedError:
        print(f"  TCP {name} - FAIL - Connection refused")
        print("  -> STM32 is reachable but port is not open.")
    except OSError as e:
        print(f"  TCP {name} - FAIL - {e}")
    return False

def main():
    print("=" * 60)
    print("STM32 W5100 Ethernet Connectivity Test")
    print("=" * 60)
    print(f"Target: {STM32_IP}")
    print(f"Control port: {CTRL_PORT}")
    print(f"Video port: {VIDEO_PORT}")
    print()

    results = []
    results.append(("Ping", test_ping()))
    results.append(("TCP 5000 (Control)", test_tcp(CTRL_PORT, "Control")))
    results.append(("TCP 5001 (Video)", test_tcp(VIDEO_PORT, "Video")))

    print("\n" + "=" * 60)
    print("RESULTS SUMMARY")
    print("=" * 60)
    for name, ok in results:
        print(f"  {name}: {'PASS' if ok else 'FAIL'}")
    
    passed = sum(1 for _, ok in results if ok)
    print(f"\n  {passed}/3 tests passed")
    
    if passed == 0:
        print("\nDiagnosis: STM32 is NOT reachable at all.")
        print("  - Check Ethernet cable is plugged into the same network as PC")
        print("  - Check W5100 SPI wiring (SCK=PA5, MISO=PA6, MOSI=PA7, CS=PB6)")
        print("  - Check firmware is flashed and running (use SWD debugger)")
    elif passed == 1:
        print("\nDiagnosis: Device responds to ping but no TCP ports open.")
        print("  - Firmware may be running but TCP server not started")
        print("  - Check UART/SWD output for W5100 init errors")
    elif passed == 2:
        print("\nDiagnosis: Device reachable, control port open, video port not.")
        print("  - Video socket may need more time to start")
        print("  - Check firmware video socket initialization")
    else:
        print("\nAll tests passed! Run the full client: python tcp_client.py")

if __name__ == "__main__":
    main()