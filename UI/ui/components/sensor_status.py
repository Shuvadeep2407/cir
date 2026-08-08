"""
MED-FLIGHT Counter-UAS Detection System
Sensor status panel with gauges
"""
from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QPainter, QColor, QPen, QFont, QBrush
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QProgressBar, QGridLayout, QPushButton)
from utils.constants import COLORS, SensorType


class SensorGauge(QWidget):
    """Single sensor status gauge widget"""
    def __init__(self, name: str, sensor_type: str, color: str, parent=None):
        super().__init__(parent)
        self.name = name
        self.sensor_type = sensor_type
        self.color = color
        self.status = "STANDBY"
        self.value = 0.0
        self.confidence = 0.0
        self.online = False
        self.cpu = 0.0
        self.temp = 35.0
        self.setMinimumSize(140, 120)
        self.setMaximumSize(200, 150)

    def update_data(self, sensor):
        self.status = sensor.status
        self.value = sensor.range_m if hasattr(sensor, 'range_m') else 0
        self.confidence = sensor.last_reading.confidence if sensor.last_reading else 0
        self.online = sensor.online
        self.cpu = sensor.cpu_usage
        self.temp = sensor.temperature_c
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        w, h = self.width(), self.height()

        # Background
        painter.fillRect(0, 0, w, h, QColor(COLORS["bg_card"]))

        # Status bar at top
        status_color = COLORS["success"] if self.online else COLORS["warning"]
        painter.fillRect(0, 0, w, 3, QColor(status_color))

        # Name
        painter.setPen(QColor(self.color))
        painter.setFont(QFont("Consolas", 9, QFont.Bold))
        painter.drawText(6, 18, self.name)

        # Type
        painter.setPen(QColor(COLORS["text_muted"]))
        painter.setFont(QFont("Consolas", 7))
        painter.drawText(6, 30, self.sensor_type)

        # Status indicator
        painter.setPen(Qt.NoPen)
        painter.setBrush(QBrush(QColor(status_color)))
        painter.drawEllipse(6, 35, 8, 8)

        painter.setPen(QColor(COLORS["text_secondary"]))
        painter.setFont(QFont("Consolas", 8))
        painter.drawText(18, 43, self.status)

        # Value
        painter.setPen(QColor(self.color))
        painter.setFont(QFont("Consolas", 16, QFont.Bold))
        val_text = f"{self.value:.0f}" if self.value > 0 else "---"
        painter.drawText(6, 70, val_text)

        # Confidence bar
        bar_y = 78
        bar_h = 4
        painter.fillRect(6, bar_y, w - 12, bar_h, QColor(COLORS["bg_input"]))
        conf_w = int((w - 12) * self.confidence)
        painter.fillRect(6, bar_y, conf_w, bar_h, QColor(self.color))

        # CPU and Temp
        painter.setPen(QColor(COLORS["text_muted"]))
        painter.setFont(QFont("Consolas", 7))
        painter.drawText(6, 95, f"CPU:{self.cpu:.0f}% | {self.temp:.0f}°C")

        # Bottom status
        if self.status == "ACTIVE":
            painter.setPen(QColor(COLORS["success"]))
            painter.drawText(6, h - 6, "● ONLINE")
        else:
            painter.setPen(QColor(COLORS["text_muted"]))
            painter.drawText(6, h - 6, "○ OFFLINE")

        painter.end()


class SensorStatusPanel(QWidget):
    """Panel showing all sensor status gauges"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(4)

        header = QHBoxLayout()
        title = QLabel("SENSOR STATUS")
        title.setObjectName("sectionTitle")
        header.addWidget(title)
        header.addStretch()

        self.all_on_btn = QPushButton("ALL ON")
        self.all_on_btn.setObjectName("successButton")
        self.all_on_btn.setFixedWidth(60)
        header.addWidget(self.all_on_btn)

        self.all_off_btn = QPushButton("ALL OFF")
        self.all_off_btn.setObjectName("dangerButton")
        self.all_off_btn.setFixedWidth(60)
        header.addWidget(self.all_off_btn)
        layout.addLayout(header)

        # Gauges grid
        self.grid = QGridLayout()
        self.grid.setSpacing(3)

        sensors = [
            ("RADAR", "AN/MPQ-64 Radar", COLORS["radar_color"]),
            ("RF", "RF-3000 Scanner", COLORS["rf_color"]),
            ("ACOUSTIC", "Audi Array M2K", COLORS["acoustic_color"]),
            ("LRF", "LRF-2200 Rangefinder", COLORS["lrf_color"]),
        ]
        self.gauges = {}
        for i, (stype, name, color) in enumerate(sensors):
            gauge = SensorGauge(name, stype, color)
            self.gauges[f"{stype}_01"] = gauge
            self.grid.addWidget(gauge, i // 2, i % 2)

        layout.addLayout(self.grid)

    def update_sensor(self, sensor_id: str, sensor):
        if sensor_id in self.gauges:
            self.gauges[sensor_id].update_data(sensor)