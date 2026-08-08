"""Maintenance page"""
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QGroupBox, QFormLayout, QProgressBar,
                               QFrame, QGridLayout)
from utils.constants import COLORS


class MaintenancePage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)
        title = QLabel("SYSTEM MAINTENANCE")
        title.setObjectName("pageTitle")
        layout.addWidget(title)

        # System health
        health_group = QGroupBox("SYSTEM HEALTH")
        health = QFormLayout(health_group)
        health.addRow("CPU Usage:", QProgressBar(value=45))
        health.addRow("Memory:", QProgressBar(value=62))
        health.addRow("Disk Usage:", QProgressBar(value=38))
        health.addRow("Sensor Uptime:", QLabel("02:34:12"))
        health.addRow("Database Size:", QLabel("12.4 MB"))
        health.addRow("Temp (Ambient):", QLabel("28°C"))
        layout.addWidget(health_group)

        # Diagnostics
        diag_group = QGroupBox("DIAGNOSTICS")
        diag_layout = QVBoxLayout(diag_group)
        diag_items = [
            ("Radar self-test", "PASS", COLORS["success"]),
            ("RF calibration", "PASS", COLORS["success"]),
            ("Acoustic array", "PASS", COLORS["success"]),
            ("LRF laser power", "MARGINAL", COLORS["warning"]),
            ("Camera focus", "PASS", COLORS["success"]),
            ("GPS sync", "PASS", COLORS["success"]),
            ("Network link", "PASS", COLORS["success"]),
            ("Database integrity", "PASS", COLORS["success"]),
        ]
        for test, result, color in diag_items:
            row = QHBoxLayout()
            row.addWidget(QLabel(test))
            row.addStretch()
            lbl = QLabel(f"[{result}]")
            lbl.setStyleSheet(f"color: {color}; font-weight: bold;")
            row.addWidget(lbl)
            diag_layout.addLayout(row)
        layout.addWidget(diag_group)

        # Actions
        actions = QHBoxLayout()
        self.self_test_btn = QPushButton("RUN SELF-TEST")
        self.self_test_btn.setObjectName("primaryButton")
        actions.addWidget(self.self_test_btn)
        self.repair_btn = QPushButton("REPAIR DATABASE")
        self.repair_btn.setObjectName("dangerButton")
        actions.addWidget(self.repair_btn)
        self.logs_btn = QPushButton("EXPORT DIAGNOSTICS")
        actions.addWidget(self.logs_btn)
        layout.addLayout(actions)
        layout.addStretch()