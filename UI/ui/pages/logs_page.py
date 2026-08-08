"""
MED-FLIGHT Counter-UAS Detection System
Event logs page
"""
from datetime import datetime
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QTableWidget, QTableWidgetItem,
                               QHeaderView, QComboBox, QDateTimeEdit, QLineEdit)
from utils.constants import COLORS


class LogsPage(QWidget):
    """System event logs viewer"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        title = QLabel("SYSTEM EVENT LOGS")
        title.setObjectName("pageTitle")
        layout.addWidget(title)

        # Filters
        filters = QHBoxLayout()
        filters.addWidget(QLabel("Type:"))
        self.type_combo = QComboBox()
        self.type_combo.addItems(["ALL", "SENSOR", "TARGET", "ALERT", "SYSTEM", "USER", "NETWORK"])
        filters.addWidget(self.type_combo)
        filters.addWidget(QLabel("Search:"))
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("Search logs...")
        self.search_input.setFixedWidth(200)
        filters.addWidget(self.search_input)
        filters.addStretch()
        self.export_btn = QPushButton("EXPORT CSV")
        self.export_btn.setObjectName("primaryButton")
        filters.addWidget(self.export_btn)
        self.clear_btn = QPushButton("CLEAR")
        self.clear_btn.setObjectName("dangerButton")
        filters.addWidget(self.clear_btn)
        layout.addLayout(filters)

        # Table
        self.table = QTableWidget()
        self.table.setColumnCount(6)
        self.table.setHorizontalHeaderLabels(["TIME", "TYPE", "SOURCE", "EVENT", "DESCRIPTION", "DATA"])
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.setAlternatingRowColors(True)
        layout.addWidget(self.table)

        # Add sample log entries
        self._add_sample_logs()

    def _add_sample_logs(self):
        import random
        sample_logs = [
            ("SENSOR", "RADAR_01", "DETECTION", "Target acquired at 1.2km", {"rcs": -12.5, "confidence": 0.94}),
            ("SENSOR", "RF_01", "SIGNAL", "RF signal detected at 2.4GHz", {"strength": -45, "modulation": "OFDM"}),
            ("TARGET", "UAS-0000", "CLASSIFIED", "Classification: DJI_MAVIC_3", {"confidence": 0.92, "threat": "LOW"}),
            ("ALERT", "SYSTEM", "WARNING", "Target altitude below 100m", {"target": "UAS-0001", "alt": 85}),
            ("SYSTEM", "SENSOR_FUSION", "FUSION", "Multi-sensor fusion completed", {"sources": ["RADAR", "RF", "ACOUSTIC"]}),
            ("USER", "OPERATOR", "LOGIN", "User operator logged in", {"ip": "192.168.1.100"}),
            ("NETWORK", "STREAM", "CONNECT", "UDP stream connected on port 5000", {}),
            ("SENSOR", "ACOUSTIC_01", "DETECTION", "Acoustic signature: QUADCOPTER", {"freq": 250, "db": 62}),
            ("SYSTEM", "AI_ENGINE", "PREDICTION", "Trajectory predicted for UAS-0000", {"steps": 15, "confidence": 0.87}),
            ("TARGET", "UAS-0002", "LOST", "Target track lost, timeout", {"last_seen": "5s ago"}),
        ]
        self.table.setRowCount(len(sample_logs))
        for i, (etype, source, event, desc, data) in enumerate(sample_logs):
            self.table.setItem(i, 0, QTableWidgetItem(
                (datetime.now().toisoformat() if hasattr(datetime.now(), 'toisoformat')
                 else datetime.now().strftime("%H:%M:%S"))))
            self.table.setItem(i, 1, QTableWidgetItem(etype))
            self.table.setItem(i, 2, QTableWidgetItem(source))
            self.table.setItem(i, 3, QTableWidgetItem(event))
            self.table.setItem(i, 4, QTableWidgetItem(desc))
            self.table.setItem(i, 5, QTableWidgetItem(str(data)))