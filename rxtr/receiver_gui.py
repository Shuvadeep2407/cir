import tkinter as tk
from tkinter import ttk
from tkinter.scrolledtext import ScrolledText
import serial
import threading
import queue
import re
import struct
import time
from datetime import datetime

# --- Configuration ---
# TODO: Change 'COM3' to the correct COM port for your receiver.
# You can find the port in the Windows Device Manager.
SERIAL_PORT = 'COM4'
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
# ---------------------

# --- Packet Definitions (from your C code) ---
RADIO_PACKET_TYPE_TELEM = 0x01
RADIO_PACKET_TYPE_LOG = 0x02
RADIO_PACKET_TYPE_AUDIO = 0x04
RADIO_DEST_ID = 11

# --- Audio Definitions ---
AUDIO_SAMPLE_RATE = 8000
AUDIO_CHANNELS = 1
AUDIO_BITS_PER_SAMPLE = 16

class SerialReaderThread(threading.Thread):
    """
    A thread for reading serial data in the background to prevent freezing the GUI.
    """
    def __init__(self, port, baudrate, data_queue):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.data_queue = data_queue
        self.serial_instance = None
        self.running = True
        # Regex to parse "Packet: <hex_data> | ... CRC OK"
        self.line_regex = re.compile(r"Packet: ([\sA-Fa-f0-9]+) \| .* (CRC OK)")
        self.log_line_regex = re.compile(r"Packet: ([\sA-Fa-f0-9]+) \| .* (CRC OK)")
        self.audio_line_regex = re.compile(r"AudioData: ID=(\d+) Angle=(\d+) Data=([A-Fa-f0-9]+)")

    def _parse_line(self, line):
        """
        Parses a single line from the serial output, verifies it,
        and extracts the relevant data.
        """
        match = self.line_regex.search(line)
        if not match:
            return None # Line doesn't match expected format
        # Try parsing as an audio data line first
        audio_match = self.audio_line_regex.search(line)
        if audio_match:
            source_id, angle, hex_data = audio_match.groups()
            try:
                audio_data = bytearray.fromhex(hex_data)
                return {"type": "audio", "source_id": int(source_id), "angle": int(angle), "audio_data": audio_data}
            except ValueError:
                return None

        hex_string, crc_status = match.groups()
        # Fallback to parsing as a standard log/telem packet
        log_match = self.log_line_regex.search(line)
        if not log_match:
            return None

        hex_string, crc_status = log_match.groups()
        # Background Verification Step 1: Check for "CRC OK"
        if crc_status != "CRC OK":
            return None

        try:
            # Convert hex string to a list of integer bytes
            payload = [int(h, 16) for h in hex_string.strip().split()]
        except ValueError:
            return None # Malformed hex data

        # Background Verification Step 2: Basic packet validation
        if len(payload) < 4:
            return None # Packet is too short
        

        dest_id = payload[0]
        source_id = payload[1]
        packet_type = payload[2]

        if dest_id != RADIO_DEST_ID:
            return None # Packet not for us

        # --- Data Extraction for GUI ---
        # The user only wants to see the 'value' from LOG packets.
        if packet_type == RADIO_PACKET_TYPE_LOG and len(payload) == 13:
            # The 'value' is the last 4 bytes (little-endian)
            value_bytes = bytearray(payload[9:13])
            # '<i' unpacks 4 bytes as a signed little-endian integer
            value = struct.unpack('<i', value_bytes)[0]
            return {"source_id": source_id, "value": f"{value} (0x{value:08X})"}

        elif packet_type == RADIO_PACKET_TYPE_TELEM:
            # For other packet types, we can just show the source ID
            return {"source_id": source_id, "value": "Telemetry Packet"}

        elif packet_type == RADIO_PACKET_TYPE_AUDIO and len(payload) > 5:
            frame_seq = payload[3]
            packet_seq = payload[4]
            audio_data = bytearray(payload[5:])
            return {"type": "audio", "source_id": source_id,
                    "frame_seq": frame_seq, "packet_seq": packet_seq,
                    "audio_data": audio_data}


        return None

    def run(self):
        """Main loop for the thread."""
        while self.running:
            try:
                self.data_queue.put({"status": "Connecting..."})
                self.serial_instance = serial.Serial(self.port, self.baudrate, timeout=1)
                self.data_queue.put({"status": f"Connected to {self.port}"})
                self.data_queue.put({"status": f"Connected to {self.port} at {self.baudrate} baud"})
                
                while self.running:
                    if self.serial_instance.in_waiting > 0:
                        line = self.serial_instance.readline().decode('utf-8', errors='ignore')
                        parsed_data = self._parse_line(line)
                        if parsed_data:
                            self.data_queue.put(parsed_data)
            except serial.SerialException:
                self.data_queue.put({"status": f"Error: Port {self.port} not found. Retrying..."})
                time.sleep(3) # Wait before retrying
            except Exception as e:
                self.data_queue.put({"status": f"An error occurred: {e}"})
                time.sleep(3)
        
        if self.serial_instance and self.serial_instance.is_open:
            self.serial_instance.close()

    def stop(self):
        self.running = False

class ReceiverApp:
    def __init__(self, root):
        self.root = root
        self.root.title("CC1101 Audio Receiver")
        self.root.geometry("450x200")
        self.root.title("Acoustic Receiver")
        self.root.geometry("450x180")

        self.data_queue = queue.Queue()
        self._setup_gui()
        self.audio_assemblers = {}

        # Start the background thread for reading serial data
        self.serial_thread = SerialReaderThread(SERIAL_PORT, BAUD_RATE, self.data_queue)
        self.serial_thread.daemon = True
        self.serial_thread.start()

        # Handle window closing
        self.root.protocol("WM_DELETE_WINDOW", self._on_closing)

        # Start the process to check the queue for new data
        self._process_serial_queue()

    def _setup_gui(self):
        frame = ttk.Frame(self.root, padding="15")
        frame = ttk.Frame(self.root, padding="10")
        frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # --- Data Display ---
        self.device_id_var = tk.StringVar(value="--")
        self.value_var = tk.StringVar(value="--")
        self.angle_var = tk.StringVar(value="--")
        self.file_saved_var = tk.StringVar(value="N/A")
        self.status_var = tk.StringVar(value="Initializing...")

        ttk.Label(frame, text="Device ID:", font=("Segoe UI", 10, "bold")).grid(column=0, row=0, sticky=tk.W, pady=2)
        ttk.Label(frame, textvariable=self.device_id_var, font=("Segoe UI", 10)).grid(column=1, row=0, sticky=tk.W, padx=10)
        ttk.Label(frame, text="Device ID:", font=("Segoe UI", 10, "bold")).grid(column=0, row=0, sticky=tk.W, pady=5)
        ttk.Label(frame, textvariable=self.device_id_var, font=("Segoe UI", 10)).grid(column=1, row=0, sticky=tk.W)

        ttk.Label(frame, text="Last Log Value:", font=("Segoe UI", 10, "bold")).grid(column=0, row=1, sticky=tk.W, pady=2)
        ttk.Label(frame, textvariable=self.value_var, font=("Segoe UI", 10)).grid(column=1, row=1, sticky=tk.W, padx=10)
        ttk.Label(frame, text="Last Log Value:", font=("Segoe UI", 10, "bold")).grid(column=0, row=1, sticky=tk.W, pady=5)
        ttk.Label(frame, textvariable=self.value_var, font=("Segoe UI", 10)).grid(column=1, row=1, sticky=tk.W)

        ttk.Label(frame, text="File Saved:", font=("Segoe UI", 10, "bold")).grid(column=0, row=2, sticky=tk.W, pady=10)
        ttk.Label(frame, textvariable=self.file_saved_var, font=("Segoe UI", 10, "italic")).grid(column=1, row=2, sticky=tk.W, padx=10)
        ttk.Label(frame, text="Last Audio Angle:", font=("Segoe UI", 10, "bold")).grid(column=0, row=2, sticky=tk.W, pady=5)
        ttk.Label(frame, textvariable=self.angle_var, font=("Segoe UI", 10)).grid(column=1, row=2, sticky=tk.W)

        ttk.Label(frame, text="File Saved:", font=("Segoe UI", 10, "bold")).grid(column=0, row=3, sticky=tk.W, pady=5)
        ttk.Label(frame, textvariable=self.file_saved_var, font=("Segoe UI", 10, "italic")).grid(column=1, row=3, sticky=tk.W)

        # --- Status Bar ---
        status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W, padding=2)
        status_bar.grid(row=1, column=0, sticky=(tk.W, tk.E))

    def _process_serial_queue(self):
        try:
            message = self.data_queue.get_nowait()
            if "status" in message:
                self.status_var.set(message["status"])
            elif "file_saved" in message:
                self.file_saved_var.set(message["file_saved"])
            elif "source_id" in message and "value" in message:
                self.device_id_var.set(str(message["source_id"]))
                self.value_var.set(str(message["value"]))
            elif message.get("type") == "audio":
                if "angle" in message:
                    self.device_id_var.set(str(message["source_id"]))
                    self.angle_var.set(f"{message['angle']} degrees")
                    self._save_wav_file(message["source_id"], message["audio_data"], angle=message["angle"])
                else:
                    self._handle_audio_packet(message)

        except queue.Empty:
            pass # No new data
        
        # Schedule this method to be called again after 100ms
        self.root.after(100, self._process_serial_queue)

    def _handle_audio_packet(self, msg):
        source_id = msg["source_id"]
        frame_seq = msg["frame_seq"]
        packet_seq = msg["packet_seq"]
        is_last = (packet_seq & 0x80) != 0
        packet_num = packet_seq & 0x7F

        # Get or create an assembler for this device
        if source_id not in self.audio_assemblers:
            self.audio_assemblers[source_id] = {"frame_seq": -1, "packets": {}}
        
        assembler = self.audio_assemblers[source_id]

        # If we get a packet from a new frame, reset the assembler
        if frame_seq != assembler["frame_seq"]:
            assembler["frame_seq"] = frame_seq
            assembler["packets"] = {}
            self.status_var.set(f"Receiving new audio frame {frame_seq} from {source_id}...")

        assembler["packets"][packet_num] = msg["audio_data"]

        if is_last:
            self.status_var.set(f"Final packet for frame {frame_seq} received. Assembling...")
            # Check if we have all packets
            expected_packets = set(range(packet_num + 1))
            received_packets = set(assembler["packets"].keys())

            if expected_packets.issubset(received_packets):
                # Assemble the full audio data in order
                full_audio_data = bytearray()
                for i in range(packet_num + 1):
                    full_audio_data.extend(assembler["packets"][i])
                
                self._save_wav_file(source_id, full_audio_data) # Angle is not available in multi-packet frames
                # Clear the assembler for the next frame
                assembler["packets"] = {}
            else:
                missing = expected_packets - received_packets
                self.status_var.set(f"Frame {frame_seq} incomplete. Missing {len(missing)} packets.")

    def _save_wav_file(self, source_id, data, angle=None):
        """Saves the raw PCM data as a .wav file."""
        timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        if angle is not None:
            filename = f"audio_{source_id}_angle_{angle}_{timestamp}.wav"
        else:
            filename = f"audio_{source_id}_{timestamp}.wav"

        num_samples = len(data) // (AUDIO_BITS_PER_SAMPLE // 8)
        
        with open(filename, 'wb') as wf:
            # --- WAV Header ---
            wf.write(b'RIFF')
            # ChunkSize
            wf.write(struct.pack('<I', 36 + len(data)))
            wf.write(b'WAVE')
            # "fmt " sub-chunk
            wf.write(b'fmt ')
            wf.write(struct.pack('<I', 16)) # Subchunk1Size for PCM
            wf.write(struct.pack('<H', 1))  # AudioFormat (1 for PCM)
            wf.write(struct.pack('<H', AUDIO_CHANNELS))
            wf.write(struct.pack('<I', AUDIO_SAMPLE_RATE))
            wf.write(struct.pack('<I', AUDIO_SAMPLE_RATE * AUDIO_CHANNELS * (AUDIO_BITS_PER_SAMPLE // 8))) # ByteRate
            wf.write(struct.pack('<H', AUDIO_CHANNELS * (AUDIO_BITS_PER_SAMPLE // 8))) # BlockAlign
            wf.write(struct.pack('<H', AUDIO_BITS_PER_SAMPLE))
            # "data" sub-chunk
            wf.write(b'data')
            wf.write(struct.pack('<I', len(data))) # Subchunk2Size
            # --- Audio Data ---
            wf.write(data)
        
        self.data_queue.put({"file_saved": filename})

    def _on_closing(self):
        print("Closing application...")
        self.serial_thread.stop()
        self.serial_thread.join(timeout=1) # Wait for thread to finish
        self.root.destroy()

if __name__ == "__main__":
    # Before running, you might need to install the serial library:
    # pip install pyserial
    
    app_root = tk.Tk()
    app = ReceiverApp(app_root)
    app_root.mainloop()
