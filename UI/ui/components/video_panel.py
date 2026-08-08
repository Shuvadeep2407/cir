"""
MED-FLIGHT Counter-UAS Detection System
4-camera video panel with quad view
"""
import numpy as np
from PySide6.QtCore import Qt, QTimer, Signal
from PySide6.QtGui import QPainter, QColor, QFont, QImage, QPixmap, QPen
from PySide6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton, QGridLayout
from core.simulators import CameraSimulator
from utils.constants import COLORS


class CameraView(QWidget):
    """Single camera view widget"""
    def __init__(self, name: str, mode: str = "EO", parent=None):
        super().__init__(parent)
        self.name = name
        self.mode = mode
        self.simulator = CameraSimulator(mode)
        self.frame = None
        self.setMinimumSize(200, 150)
        self.setStyleSheet(f"""
            CameraView {{
                background-color: #0a0e14;
                border: 1px solid {COLORS["border"]};
                border-radius: 4px;
            }}
        """)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(2, 2, 2, 2)

        # Header
        header = QHBoxLayout()
        self.label = QLabel(f"{'📷' if mode != 'IR' else '🌡'} {name}")
        self.label.setStyleSheet(f"color: {COLORS['accent_cyan']}; font-size: 10px; font-weight: bold;")
        header.addWidget(self.label)
        header.addStretch()

        self.status_led = QLabel("●")
        self.status_led.setStyleSheet(f"color: {COLORS['success']}; font-size: 8px;")
        header.addWidget(self.status_led)
        layout.addLayout(header)

    def update_frame(self, frame: np.ndarray = None):
        if frame is not None:
            self.frame = frame
        else:
            self.frame = self.simulator.get_frame()
        self.update()

    def paintEvent(self, event):
        if self.frame is None:
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        h, w = self.frame.shape[:2]
        img_h = self.height() - 30
        img_w = self.width() - 4
        scale = min(img_w / w, img_h / h)
        nw, nh = int(w * scale), int(h * scale)
        x_off = (self.width() - nw) // 2
        y_off = 26

        # Convert frame to QImage
        if len(self.frame.shape) == 2:
            qimg = QImage(self.frame.data, w, h, w, QImage.Format_Grayscale8)
        else:
            rgb = self.frame[..., ::-1]  # BGR to RGB
            qimg = QImage(rgb.data, w, h, 3 * w, QImage.Format_RGB888)

        pixmap = QPixmap.fromImage(qimg).scaled(nw, nh, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        painter.drawPixmap(x_off, y_off, pixmap)

        # Overlay text
        painter.setPen(QColor(COLORS["accent_green"]))
        painter.setFont(QFont("Consolas", 8))
        painter.drawText(x_off + 4, y_off + 14, f"{self.mode} | {nw}x{nh}")

        painter.end()


class VideoPanel(QWidget):
    """4-camera video panel in 2x2 grid"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()
        self._update_timer = QTimer(self)
        self._update_timer.timeout.connect(self._update_frames)
        self._update_timer.start(100)

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(2)

        # Header
        header = QHBoxLayout()
        title = QLabel("VIDEO FEEDS")
        title.setObjectName("sectionTitle")
        header.addWidget(title)
        header.addStretch()

        self.record_btn = QPushButton("● REC")
        self.record_btn.setObjectName("dangerButton")
        self.record_btn.setFixedWidth(70)
        header.addWidget(self.record_btn)

        self.layout_btn = QPushButton("2x2")
        self.layout_btn.setFixedWidth(50)
        header.addWidget(self.layout_btn)
        layout.addLayout(header)

        # Camera grid
        self.grid = QGridLayout()
        self.grid.setSpacing(2)

        self.cameras = [
            CameraView("EO CAM-1", "EO"),
            CameraView("IR CAM-1", "IR"),
            CameraView("EO CAM-2", "EO"),
            CameraView("IR CAM-2", "IR"),
        ]

        positions = [(0, 0), (0, 1), (1, 0), (1, 1)]
        for cam, pos in zip(self.cameras, positions):
            self.grid.addWidget(cam, pos[0], pos[1])
        layout.addLayout(self.grid)

    def _update_frames(self):
        for cam in self.cameras:
            cam.update_frame()

    def toggle_recording(self):
        btn = self.record_btn
        if "REC" in btn.text():
            btn.setText("■ STOP")
            btn.setStyleSheet(f"background-color: {COLORS['accent_red']}; color: white;")
        else:
            btn.setText("● REC")
            btn.setStyleSheet("")