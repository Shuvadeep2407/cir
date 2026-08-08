"""Mission playback page"""
from pathlib import Path
from PySide6.QtCore import Qt, QTimer
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QListWidget, QSlider, QFrame,
                               QFileDialog, QMessageBox)
from utils.constants import COLORS


class PlaybackPage(QWidget):
    def __init__(self, playback_engine=None, parent=None):
        super().__init__(parent)
        self.playback = playback_engine
        self.setup_ui()
        if playback_engine:
            playback_engine.playback_progress.connect(self._update_progress)

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)
        title = QLabel("MISSION PLAYBACK")
        title.setObjectName("pageTitle")
        layout.addWidget(title)

        content = QHBoxLayout()

        # Recording list
        list_panel = QVBoxLayout()
        list_panel.addWidget(QLabel("Available Recordings"))
        self.recording_list = QListWidget()
        self.recording_list.setMinimumWidth(250)
        list_panel.addWidget(self.recording_list)

        refresh_btn = QPushButton("REFRESH")
        refresh_btn.clicked.connect(self._refresh_list)
        list_panel.addWidget(refresh_btn)
        content.addLayout(list_panel)

        # Controls
        controls = QVBoxLayout()
        controls.addWidget(QLabel("Playback Controls"))

        # Time display
        time_display = QHBoxLayout()
        self.current_time = QLabel("00:00")
        self.current_time.setStyleSheet(f"color: {COLORS['accent_cyan']}; font-size: 20px; font-weight: bold;")
        time_display.addWidget(self.current_time)
        time_display.addWidget(QLabel("/"))
        self.total_time = QLabel("00:00")
        self.total_time.setStyleSheet(f"color: {COLORS['text_secondary']}; font-size: 20px;")
        time_display.addWidget(self.total_time)
        time_display.addStretch()
        controls.addLayout(time_display)

        # Seek bar
        self.seek_bar = QSlider(Qt.Horizontal)
        self.seek_bar.setRange(0, 1000)
        controls.addWidget(self.seek_bar)

        # Buttons
        btn_row = QHBoxLayout()
        self.load_btn = QPushButton("LOAD")
        self.load_btn.setObjectName("primaryButton")
        self.load_btn.clicked.connect(self._load_recording)
        btn_row.addWidget(self.load_btn)

        self.play_btn = QPushButton("▶ PLAY")
        self.play_btn.setObjectName("successButton")
        self.play_btn.clicked.connect(self._toggle_playback)
        btn_row.addWidget(self.play_btn)

        self.stop_btn = QPushButton("■ STOP")
        self.stop_btn.setObjectName("dangerButton")
        self.stop_btn.clicked.connect(self._stop)
        btn_row.addWidget(self.stop_btn)

        btn_row.addWidget(QLabel("Speed:"))
        self.speed_btn = QPushButton("1.0x")
        self.speed_btn.clicked.connect(self._cycle_speed)
        btn_row.addWidget(self.speed_btn)
        controls.addLayout(btn_row)

        controls.addStretch()
        content.addLayout(controls)
        layout.addLayout(content)

    def _refresh_list(self):
        # Check if playack engine has a recordings list
        recordings = []
        if hasattr(self.playback, 'get_recordings_list'):
            recordings = self.playback.get_recordings_list()
        self.recording_list.clear()
        for r in recordings:
            dur = f"{int(r.get('duration', 0) // 60):02d}:{int(r.get('duration', 0) % 60):02d}"
            self.recording_list.addItem(
                f"{r.get('recording_id', 'Unknown')} ({dur}) - {r.get('size_kb', 0)}KB")

    def _load_recording(self):
        if not self.playback:
            return
        filepath, _ = QFileDialog.getOpenFileName(
            self, "Load Recording", str(Path("recordings")), "MFR Files (*.mfr)")
        if filepath:
            if self.playback.load(filepath):
                self.total_time.setText(self.playback.get_duration_str())
                self._refresh_list()
            else:
                QMessageBox.warning(self, "Error", "Failed to load recording file")

    def _toggle_playback(self):
        if not self.playback or not self.playback.data:
            return
        if self.playback.is_playing:
            self.playback.pause()
            self.play_btn.setText("▶ PLAY")
        else:
            self.playback.play()
            self.play_btn.setText("⏸ PAUSE")

    def _stop(self):
        if self.playback:
            self.playback.stop()
            self.play_btn.setText("▶ PLAY")
            self.current_time.setText("00:00")
            self.seek_bar.setValue(0)

    def _cycle_speed(self):
        speeds = [0.5, 1.0, 2.0, 5.0, 10.0]
        current = float(self.speed_btn.text().replace("x", ""))
        idx = (speeds.index(current) + 1) % len(speeds) if current in speeds else 1
        self.speed_btn.setText(f"{speeds[idx]}x")
        if self.playback:
            self.playback.set_speed(speeds[idx])

    def _update_progress(self, progress):
        self.seek_bar.setValue(int(progress * 1000))
        if self.playback:
            self.current_time.setText(self.playback.get_current_time_str())