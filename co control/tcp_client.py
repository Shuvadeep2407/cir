# tcp_client.py
# STM32 W5100 Ethernet Test Client
# Connects to STM32 on two channels:
#   Port 5000 - Control & Debug (echo test, status)
#   Port 5001 - Video data (receives video frames)

import socket
import threading
import time
import sys

# --- Configuration ---
STM32_IP = "10.87.243.100"  # The static IP on the STM32
CTRL_PORT = 5000            # Control & Debug channel
VIDEO_PORT = 5001           # Video channel

# --- Connection state ---
ctrl_connected = False
video_connected = False
video_frame_count = 0
stop_event = threading.Event()

def receive_ctrl(sock, name):
    """Receive thread for the control/debug channel."""
    global ctrl_connected
    print(f"[{name}] Receiver started")
    while not stop_event.is_set():
        try:
            sock.settimeout(1.0)
            data = sock.recv(2048)
            if not data:
                print(f"\n[{name}] Connection closed by server")
                break
            print(f"[{name}] Rx: {data.decode('utf-8', errors='replace')}", end='')
        except socket.timeout:
            continue
        except (ConnectionResetError, BrokenPipeError, OSError) as e:
            print(f"\n[{name}] Connection lost: {e}")
            break
    ctrl_connected = False
    print(f"[{name}] Receiver finished")

def receive_video(sock, name):
    """Receive thread for the video channel."""
    global video_connected, video_frame_count
    print(f"[{name}] Receiver started")
    while not stop_event.is_set():
        try:
            sock.settimeout(1.0)
            data = sock.recv(4096)
            if not data:
                print(f"\n[{name}] Connection closed by server")
                break
            video_frame_count += 1
            # Print frame info (truncate large binary data)
            text = data.decode('utf-8', errors='replace')
            print(f"[{name}] Frame #{video_frame_count}: {text.strip()[:80]}")
        except socket.timeout:
            continue
        except (ConnectionResetError, BrokenPipeError, OSError) as e:
            print(f"\n[{name}] Connection lost: {e}")
            break
    video_connected = False
    print(f"[{name}] Receiver finished")

def connect_with_retry(ip, port, name, timeout=5):
    """Connect to the STM32 with retry."""
    print(f"[{name}] Connecting to {ip}:{port}...")
    while not stop_event.is_set():
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(timeout)
            s.connect((ip, port))
            print(f"[{name}] Connected!")
            return s
        except (socket.timeout, ConnectionRefusedError, OSError) as e:
            print(f"[{name}] Connection failed: {e}. Retrying in 2s...")
            s.close()
            time.sleep(2)
    return None

def ctrl_loop(sock):
    """Main control channel loop - send commands and receive responses."""
    global ctrl_connected
    ctrl_connected = True
    receiver = threading.Thread(target=receive_ctrl, args=(sock, "CTRL"), daemon=True)
    receiver.start()

    print("\n=== Control Channel Ready ===")
    print("Commands: type text to send, 'status' for info, 'quit' to exit")
    try:
        while ctrl_connected and not stop_event.is_set():
            try:
                cmd = input(">> ")
            except EOFError:
                break
            if cmd.lower() == 'quit':
                break
            if cmd.lower() == 'status':
                print(f"[CTRL] Status: connected={ctrl_connected}, video_frames={video_frame_count}")
                continue
            try:
                sock.sendall(cmd.encode('utf-8'))
                print(f"[CTRL] Sent: {cmd}")
            except (ConnectionResetError, BrokenPipeError, OSError) as e:
                print(f"[CTRL] Send failed: {e}")
                break
    finally:
        print("[CTRL] Closing control channel")
        try:
            sock.close()
        except:
            pass

def main():
    """Main entry point."""
    print("=== STM32 W5100 Ethernet Test Client ===")
    print(f"Target: {STM32_IP}")
    print(f"Control port: {CTRL_PORT}")
    print(f"Video port: {VIDEO_PORT}")
    print("Press Ctrl+C to exit\n")

    # Connect to control channel (blocking with retry)
    ctrl_sock = connect_with_retry(STM32_IP, CTRL_PORT, "CTRL")
    if ctrl_sock is None:
        print("[CTRL] Could not connect. Exiting.")
        return

    # Try to connect to video channel (non-blocking, warn if fails)
    video_sock = None
    try:
        video_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        video_sock.settimeout(3)
        video_sock.connect((STM32_IP, VIDEO_PORT))
        print("[VIDEO] Connected!")
        video_connected = True
        threading.Thread(target=receive_video, args=(video_sock, "VIDEO"), daemon=True).start()
    except (socket.timeout, ConnectionRefusedError, OSError) as e:
        print(f"[VIDEO] Could not connect (video channel may be starting): {e}")
        video_sock = None

    # Run control loop
    try:
        ctrl_loop(ctrl_sock)
    except KeyboardInterrupt:
        print("\n[MAIN] Ctrl+C pressed")
    finally:
        stop_event.set()
        print("\n[MAIN] Shutting down...")
        if video_sock:
            try:
                video_sock.close()
            except:
                pass
        time.sleep(0.5)
        print("[MAIN] Done.")

if __name__ == "__main__":
    main()