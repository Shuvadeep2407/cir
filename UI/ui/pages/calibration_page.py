"""Calibration page"""
from PySide6.QtCore import Qt, QTimer
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QProgressBar, QGroupBox, QGridLayout,
                               QFrame, QSlider)
from utils.constants import COLORS


class CalibrationPage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)
        title = QLabel("SENSOR CALIBRATION")
        title.setObjectName("pageTitle")
        layout.addWidget(title)

        # Sensor calibration cards
        grid = QGridLayout()
        grid.setSpacing(8)
        sensors = [
            ("RADAR", "AN/MPQ-64", 87, "Calibrated"),
            ("RF SCANNER", "RF-3000", 65, "Needs tuning"),
            ("ACOUSTIC", "Audi M2K", 92, "Calibrated"),
            ("LRF", "LRF-2200", 45, "Out of spec"),
        ]
        for i, (name, model, cal_pct, status) in enumerate(sensors):
            card = QFrame()
            card.setStyleSheet(f"background-color: {COLORS['bg_card']}; border: 1px solid {COLORS['border']}; border-radius: 6px;")
            cl = QVBoxLayout(card)
            cl.addWidget(QLabel(f"<b>{name}</b>"))
            cl.addWidget(QLabel(f"Model: {model}"))
            pb = QProgressBar()
            pb.setValue(cal_pct)
            cl.addWidget(pb)
            status_lbl = QLabel(f"Status: {status}")
            color_key = 'success' if cal_pct > 70 else ('warning' if cal_pct > 50 else 'danger')
            status_lbl.setStyleSheet(f"color: {COLORS[color_key]}")
            cl.addWidget(status_lbl)
            cal_btn = QPushButton("CALIBRATE")
            cal_btn.setObjectName("primaryButton")
            cl.addWidget(cal_btn)
            grid.addWidget(card, i // 2, i % 2)
        layout.addLayout(grid)

        # System calibration
        sys_group = QGroupBox("SYSTEM CALIBRATION")
        sys_layout = QVBoxLayout(sys_group)
        sys_layout.addWidget(QLabel("Sensor alignment: Complete"))
        sys_layout.addWidget(QLabel("GPS reference: Locked"))
        sys_layout.addWidget(QLabel("Magnetic declination: 12.3°E"))
        align_pb = QProgressBar()
        align_pb.setValue(100)
        sys_layout.addWidget(align_pb)
        sys_layout.addWidget(QPushButton("RUN FULL SYSTEM CALIBRATION"))
        layout.addWidget(sys_group)
        layout.addStretch()