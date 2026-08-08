"""
MED-FLIGHT Counter-UAS Detection System
Playback engine - replays recorded mission data
"""
import time
import pickle
from datetime import datetime, timedelta
from collections import defaultdict
from PySide6.QtCore import QObject, Signal, QTimer
from utils.logger import log


class PlaybackEngine(QObject):
    """Replays recorded mission data with time controls"""
    playback_tick = Signal(float, dict)  # elapsed_seconds, data_snapshot
    playback_started = Signal()
    playback_paused = Signal()
    playback_stopped = Signal()
    playback_finished = Signal()
    playback_progress = Signal(float)  # 0.0 to 1.0

    def __init__(self, parent=None):
        super().__init__(parent)
        self.data = None
        self.is_playing = False
        self.is_paused = False
        self.speed = 1.0
        self.current_time = 0.0
        self.duration = 0.0
        self._timer = QTimer(self)
        self._timer.timeout.connect(self._tick)
        self._start_time = None
        self._all_events = []

    def load(self, filepath: str) -> bool:
        try:
            with open(filepath, "rb") as f:
                self.data = pickle.load(f)
            header = self.data.get("header", [{}])[0]
            start = header.get("start_time", "")
            end = header.get("end_time", "")
            if start and end:
                start_dt = datetime.fromisoformat(start)
                end_dt = datetime.fromisoformat(end)
                self.duration = (end_dt - start_dt).total_seconds()
            else:
                self.duration = header.get("duration_seconds", 60)

            # Flatten all events with timestamps
            self._all_events = []
            for category, events in self.data.items():
                if category == "header":
                    continue
                for event in events:
                    ts = event.get("timestamp", start)
                    try:
                        evt_time = (datetime.fromisoformat(ts) - start_dt).total_seconds()
                    except:
                        evt_time = 0
                    self._all_events.append((evt_time, category, event))
            self._all_events.sort(key=lambda x: x[0])

            log.info(f"Loaded recording: {filepath} ({self.duration:.0f}s, {len(self._all_events)} events)")
            return True
        except Exception as e:
            log.error(f"Playback load failed: {e}")
            return False

    def play(self):
        if not self.data:
            return
        self.is_playing = True
        self.is_paused = False
        self._start_time = time.time() - (self.current_time / self.speed)
        self._timer.start(50)
        self.playback_started.emit()
        log.info(f"Playback started at {self.speed}x")

    def pause(self):
        self.is_paused = not self.is_paused
        if self.is_paused:
            self._timer.stop()
            self.playback_paused.emit()
        else:
            self._start_time = time.time() - (self.current_time / self.speed)
            self._timer.start(50)
            log.info("Playback resumed")

    def stop(self):
        self.is_playing = False
        self.is_paused = False
        self.current_time = 0.0
        self._timer.stop()
        self.playback_stopped.emit()
        log.info("Playback stopped")

    def set_speed(self, speed: float):
        self.speed = max(0.1, min(10.0, speed))
        if self.is_playing and not self.is_paused:
            self._start_time = time.time() - (self.current_time / self.speed)

    def seek(self, position: float):
        self.current_time = max(0, min(self.duration, position))

    def _tick(self):
        if not self.is_playing or self.is_paused:
            return
        elapsed = time.time() - self._start_time
        self.current_time = elapsed * self.speed

        if self.current_time >= self.duration:
            self.stop()
            self.playback_finished.emit()
            return

        # Collect events up to current time
        snapshot = defaultdict(list)
        for evt_time, category, event in self._all_events:
            if evt_time <= self.current_time:
                snapshot[category].append(event)

        self.playback_tick.emit(self.current_time, dict(snapshot))
        self.playback_progress.emit(self.current_time / self.duration)

    def get_duration_str(self) -> str:
        mins = int(self.duration // 60)
        secs = int(self.duration % 60)
        return f"{mins:02d}:{secs:02d}"

    def get_current_time_str(self) -> str:
        mins = int(self.current_time // 60)
        secs = int(self.current_time % 60)
        return f"{mins:02d}:{secs:02d}"