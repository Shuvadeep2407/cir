"""
MED-FLIGHT Counter-UAS Detection System
Network stream handler for external data sources
"""
import json
import socket
import threading
from datetime import datetime
from PySide6.QtCore import QObject, Signal
from utils.logger import log


class StreamHandler(QObject):
    """Handles network data streams from external sensors"""
    data_received = Signal(str, dict)  # source, data
    connection_changed = Signal(str, bool)  # source, connected
    error_occurred = Signal(str, str)  # source, error

    def __init__(self, parent=None):
        super().__init__(parent)
        self.streams = {}
        self._running = False

    def start_udp_listener(self, name: str, port: int, host: str = "0.0.0.0"):
        """Start a UDP listener for external data"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.bind((host, port))
            sock.settimeout(1.0)
            
            thread = threading.Thread(target=self._udp_listen, args=(name, sock), daemon=True)
            self.streams[name] = {"socket": sock, "thread": thread, "type": "udp", "port": port}
            thread.start()
            self.connection_changed.emit(name, True)
            log.info(f"UDP listener started: {name} on port {port}")
        except Exception as e:
            log.error(f"Failed to start UDP listener {name}: {e}")
            self.error_occurred.emit(name, str(e))

    def _udp_listen(self, name: str, sock: socket.socket):
        while self._running:
            try:
                data, addr = sock.recvfrom(65535)
                try:
                    parsed = json.loads(data.decode("utf-8"))
                    self.data_received.emit(name, parsed)
                except json.JSONDecodeError:
                    self.data_received.emit(name, {"raw": data.hex(), "source": addr[0]})
            except socket.timeout:
                continue
            except Exception as e:
                if self._running:
                    log.warning(f"UDP {name} receive error: {e}")

    def start_tcp_client(self, name: str, host: str, port: int):
        """Connect to a TCP data source"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((host, port))
            sock.settimeout(5.0)
            
            thread = threading.Thread(target=self._tcp_listen, args=(name, sock), daemon=True)
            self.streams[name] = {"socket": sock, "thread": thread, "type": "tcp", "host": host, "port": port}
            thread.start()
            self.connection_changed.emit(name, True)
            log.info(f"TCP connected: {name} -> {host}:{port}")
        except Exception as e:
            log.error(f"TCP connection failed {name}: {e}")
            self.error_occurred.emit(name, str(e))

    def _tcp_listen(self, name: str, sock: socket.socket):
        buffer = b""
        while self._running:
            try:
                data = sock.recv(4096)
                if not data:
                    break
                buffer += data
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    if line.strip():
                        try:
                            parsed = json.loads(line.decode("utf-8"))
                            self.data_received.emit(name, parsed)
                        except json.JSONDecodeError:
                            pass
            except socket.timeout:
                continue
            except Exception as e:
                if self._running:
                    log.warning(f"TCP {name} error: {e}")
                break
        self.connection_changed.emit(name, False)
        log.info(f"TCP disconnected: {name}")

    def send_data(self, name: str, data: dict):
        """Send data through an active stream"""
        stream = self.streams.get(name)
        if not stream:
            return
        try:
            payload = (json.dumps(data) + "\n").encode("utf-8")
            if stream["type"] == "udp":
                stream["socket"].sendto(payload, ("127.0.0.1", stream["port"]))
            elif stream["type"] == "tcp":
                stream["socket"].sendall(payload)
        except Exception as e:
            log.error(f"Send failed on {name}: {e}")

    def stop_all(self):
        self._running = False
        for name, stream in self.streams.items():
            try:
                stream["socket"].close()
            except:
                pass
        self.streams.clear()
        log.info("All network streams stopped")

    def start(self):
        self._running = True

    def stop(self):
        self._running = False