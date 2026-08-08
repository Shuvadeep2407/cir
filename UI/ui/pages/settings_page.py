"""
MED-FLIGHT Counter-UAS Detection System
Settings page
"""
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QGroupBox, QFormLayout, QLineEdit,
                               QSpinBox, QComboBox, QCheckBox, QTabWidget,
                               QScrollArea)
from utils.constants import COLORS


class SettingsPage(QWidget):
    """System configuration settings"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        title = QLabel("SYSTEM SETTINGS")
        title.setObjectName("pageTitle")
        layout.addWidget(title)

        tabs = QTabWidget()

        # General tab
        general = QWidget()
        gl = QFormLayout(general)
        gl.addRow("System Name:", QLineEdit("MED-FLIGHT C-UAS-01"))
        gl.addRow("Operator ID:", QLineEdit("OP-001"))
        gl.addRow("Location:", QLineEdit("Los Angeles, CA"))
        gl.addRow("Auto-start sensors:", QCheckBox())
        gl.addRow("Default map zoom:", QSpinBox(value=14, minimum=1, maximum=20))
        gl.addRow("Data retention (days):", QSpinBox(value=90, minimum=1, maximum=365))
        tabs.addTab(general, "GENERAL")

        # Sensors tab
        sensors = QWidget()
        sl = QFormLayout(sensors)
        sl.addRow("Radar range (m):", QSpinBox(value=5000, minimum=100, maximum=20000, singleStep=100))
        sl.addRow("RF scan band (MHz):", QComboBox())
        sl.addRow("Acoustic sensitivity:", QSpinBox(value=40, minimum=10, maximum=100))
        sl.addRow("LRF max range (m):", QSpinBox(value=3000, minimum=100, maximum=10000))
        sl.addRow("Camera framerate (fps):", QSpinBox(value=30, minimum=5, maximum=60))
        sl.addRow("Enable sensor fusion:", QCheckBox(checked=True))
        tabs.addTab(sensors, "SENSORS")

        # AI tab
        ai = QWidget()
        al = QFormLayout(ai)
        al.addRow("Auto-classification:", QCheckBox(checked=True))
        al.addRow("Trajectory prediction:", QCheckBox(checked=True))
        al.addRow("Confidence threshold:", QSpinBox(value=70, minimum=0, maximum=100, suffix="%"))
        al.addRow("YOLO detection interval:", QSpinBox(value=100, minimum=33, maximum=1000, suffix="ms"))
        tabs.addTab(ai, "AI ENGINE")

        # Network tab
        net = QWidget()
        nl = QFormLayout(net)
        nl.addRow("UDP Listen Port:", QSpinBox(value=5000, minimum=1024, maximum=65535))
        nl.addRow("TCP Stream Host:", QLineEdit("192.168.1.100"))
        nl.addRow("TCP Stream Port:", QSpinBox(value=5001, minimum=1024, maximum=65535))
        nl.addRow("Enable data sharing:", QCheckBox())
        tabs.addTab(net, "NETWORK")

        # Display tab
        display = QWidget()
        dl = QFormLayout(display)
        dl.addRow("Theme:", QComboBox())
        dl.addRow("Font size:", QSpinBox(value=12, minimum=8, maximum=24))
        dl.addRow("Show grid overlay:", QCheckBox(checked=True))
        dl.addRow("Target trail length:", QSpinBox(value=120, minimum=10, maximum=500))
        dl.addRow("Radar sweep speed:", QComboBox())
        tabs.addTab(display, "DISPLAY")

        layout.addWidget(tabs)

        # Save button
        btn_layout = QHBoxLayout()
        btn_layout.addStretch()
        save_btn = QPushButton("SAVE SETTINGS")
        save_btn.setObjectName("primaryButton")
        save_btn.setFixedWidth(200)
        btn_layout.addWidget(save_btn)
        reset_btn = QPushButton("RESET TO DEFAULT")
        reset_btn.setObjectName("dangerButton")
        btn_layout.addWidget(reset_btn)
        layout.addLayout(btn_layout)