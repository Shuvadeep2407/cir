"""
MED-FLIGHT Counter-UAS Detection System
Mission recorder - records all system data for later playback
"""
import json
import pickle
import numpy as np
from pathlib import Path
from datetime import datetime
from collections import defaultdict
from PySide6.QtCore import QObject, Signal
from utils.logger import log
from utils.constants import APP_VERSION


class MissionRecorder(QObject):
    """Records mission data to disk for replay and analysis"""
    recording_started = Signal(str)  # recording_id
    recording_stopped = Signal(str)  # recording_id
    recording_error = Signal(str)  # error message

    def __init__(self, parent=None):
        super().__init__(parent)
        self.recording = False
        self.recording_id = ""
        self.recordings_dir = Path("recordings")
        self.recordings_dir.mkdir(exist_ok=True)
        self._data = defaultdict(list)
        self._start_time = None

    def start_recording(self, mission_id: str = ""):
        """Start recording mission data"""
        self.recording_id = f"MISSION_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        self._start_time = datetime.now()
        self.recording = True
        self._data = defaultdict(list)

        # Record header
        self._data["header"] = [{
            "recording_id": self.recording_id,
            "start_time": self._start_time.isoformat(),
            "app_version": APP_VERSION,
            "mission_id": mission_id,
        }]
        log.info(f"Recording started: {self.recording_id}")
        self.recording_started.emit(self.recording_id)

    def stop_recording(self):
        """Stop recording and save to disk"""
        if not self.recording:
            return
        self.recording = False
        end_time = datetime.now()

        # Add end time to header
        if self._data.get("header"):
            self._data["header"][0]["end_time"] = end_time.isoformat()
            self._data["header"][0]["duration_seconds"] = (end_time - self._start_time).total_seconds()

        # Save to file
        filepath = self.recordings_dir / f"{self.recording_id}.mfr"
        try:
            with open(filepath, "wb") as f:
                pickle.dump(dict(self._data), f)
            log.info(f"Recording saved: {filepath} ({filepath.stat().st_size / 1024:.1f} KB)")
            self.recording_stopped.emit(self.recording_id)
        except Exception as e:
            log.error(f"Failed to save recording: {e}")
            self.recording_error.emit(str(e))

    def record_event(self, category: str, data: dict):
        """Record a timestamped event"""
        if not self.recording:
            return
        entry = {
            "timestamp": datetime.now().isoformat(),
            **data,
        }
        self._data[category].append(entry)

    def record_sensor_reading(self, sensor_id: str, reading: dict):
        self.record_event(f"sensor_{sensor_id}", reading)

    def record_target_update(self, target: dict):
        self.record_event("targets", target)

    def record_alert(self, alert: dict):
        self.record_event("alerts", alert)

    def record_system_event(self, event_type: str, description: str, data: dict = None):
        entry = {"event_type": event_type, "description": description}
        if data:
            entry["data"] = data
        self.record_event("system_events", entry)

    def get_recordings_list(self) -> list:
        """List all saved recordings"""
        recordings = []
        for f in sorted(self.recordings_dir.glob("*.mfr"), reverse=True):
            try:
                with open(f, "rb") as pf:
                    data = pickle.load(pf)
                header = data.get("header", [{}])[0]
                recordings.append({
                    "file": str(f),
                    "recording_id": header.get("recording_id", f.stem),
                    "start_time": header.get("start_time", ""),
                    "end_time": header.get("end_time", ""),
                    "duration": header.get("duration_seconds", 0),
                    "size_kb": round(f.stat().st_size / 1024, 1),
                    "mission_id": header.get("mission_id", ""),
                })
            except Exception as e:
                log.warning(f"Corrupt recording file: {f} - {e}")
        return recordings

    def load_recording(self, filepath: str) -> dict:
        """Load a recording file for playback"""
        try:
            with open(filepath, "rb") as f:
                data = pickle.load(f)
            return data
        except Exception as e:
            log.error(f"Failed to load recording: {e}")
            return {}

    def delete_recording(self, filepath: str):
        path = Path(filepath)
        if path.exists():
            path.unlink()
            log.info(f"Deleted recording: {filepath}")