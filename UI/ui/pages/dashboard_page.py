"""
MED-FLIGHT Counter-UAS Detection System
Main dashboard page
"""
from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QFont, QColor
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QGridLayout, QPushButton, QFrame, QProgressBar,
                               QSplitter)
from ui.components.tactical_map import TacticalMapWidget
from ui.components.target_list import TargetListWidget
from ui.components.video_panel import VideoPanel
from ui.components.sensor_status import SensorStatusPanel
from utils.constants import COLORS


class ThreatSummaryCard(QFrame):
    """Threat level summary card"""
    def __init__(self, title, value, color, subtitle="", parent=None):
        super().__init__(parent)
        self.setStyleSheet(f"""
            ThreatSummaryCard {{
                background-color: {COLORS["bg_card"]};
                border: 1px solid {COLORS["border"]};
                border-radius: 6px;
                border-top: 3px solid {color};
            }}
        """)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 8, 12, 8)
        
        title_lbl = QLabel(title)
        title_lbl.setStyleSheet(f"color: {COLORS['text_muted']}; font-size: 10px;")
        layout.addWidget(title_lbl)
        
        self.value_lbl = QLabel(str(value))
        self.value_lbl.setStyleSheet(f"color: {color}; font-size: 28px; font-weight: bold;")
        layout.addWidget(self.value_lbl)
        
        if subtitle:
            sub_lbl = QLabel(subtitle)
            sub_lbl.setStyleSheet(f"color: {COLORS['text_secondary']}; font-size: 9px;")
            layout.addWidget(sub_lbl)

    def set_value(self, value):
        self.value_lbl.setText(str(value))


class SystemStatusBar(QFrame):
    """Top status bar showing system health"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setStyleSheet(f"""
            SystemStatusBar {{
                background-color: {COLORS["bg_secondary"]};
                border: 1px solid {COLORS["border"]};
                border-radius: 4px;
            }}
        """)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(12, 6, 12, 6)

        items = [
            ("SYS", "ACTIVE", COLORS["success"]),
            ("SENSORS", "9/9", COLORS["success"]),
            ("NET", "LINK", COLORS["success"]),
            ("GPS", "LOCK", COLORS["success"]),
            ("REC", "STANDBY", COLORS["warning"]),
            ("UPTIME", "02:34:12", COLORS["text_secondary"]),
        ]
        for label, value, color in items:
            item = QLabel(f"<b style='color:{color}'>{label}</b> {value}")
            item.setStyleSheet(f"color: {COLORS['text_secondary']}; font-size: 10px; padding: 0 8px;")
            layout.addWidget(item)
            if items.index((label, value, color)) < len(items) - 1:
                sep = QLabel("|")
                sep.setStyleSheet(f"color: {COLORS['border']};")
                layout.addWidget(sep)
        layout.addStretch()

        clock = QLabel("2026-06-24 11:32 Z")
        clock.setStyleSheet(f"color: {COLORS['text_secondary']}; font-size: 10px;")
        layout.addWidget(clock)


class DashboardPage(QWidget):
    """Main operational dashboard"""
    def __init__(self, target_manager=None, sensor_manager=None, alert_manager=None, parent=None):
        super().__init__(parent)
        self.target_manager = target_manager
        self.sensor_manager = sensor_manager
        self.alert_manager = alert_manager
        self.setup_ui()
        self._update_timer = QTimer(self)
        self._update_timer.timeout.connect(self._update_data)
        self._update_timer.start(500)

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        # Status bar
        self.status_bar = SystemStatusBar()
        layout.addWidget(self.status_bar)

        # Top row: Threat summary cards
        cards = QHBoxLayout()
        self.threat_cards = {
            "critical": ThreatSummaryCard("CRITICAL", "0", COLORS["accent_red"], "Immediate action"),
            "high": ThreatSummaryCard("HIGH", "0", COLORS["accent_orange"], "Priority tracking"),
            "medium": ThreatSummaryCard("MEDIUM", "0", COLORS["accent_yellow"], "Monitor"),
            "total": ThreatSummaryCard("TOTAL", "0", COLORS["accent_cyan"], "Active tracks"),
        }
        for card in self.threat_cards.values():
            card.setMinimumHeight(70)
            cards.addWidget(card)
        layout.addLayout(cards)

        # Main content splitter
        splitter = QSplitter(Qt.Horizontal)

        # Left: Tactical map
        map_container = QWidget()
        map_layout = QVBoxLayout(map_container)
        map_layout.setContentsMargins(0, 0, 0, 0)
        self.tactical_map = TacticalMapWidget()
        map_layout.addWidget(self.tactical_map)
        splitter.addWidget(map_container)

        # Right panel: Video + sensors
        right_panel = QWidget()
        right_layout = QVBoxLayout(right_panel)
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(4)

        self.video_panel = VideoPanel()
        right_layout.addWidget(self.video_panel)

        self.sensor_status = SensorStatusPanel()
        right_layout.addWidget(self.sensor_status)
        splitter.addWidget(right_panel)

        splitter.setStretchFactor(0, 3)
        splitter.setStretchFactor(1, 2)
        layout.addWidget(splitter, 1)

    def _update_data(self):
        if self.target_manager:
            targets = self.target_manager.get_all_targets()
            self.tactical_map.set_targets(targets)

            # Update threat cards
            summary = self.target_manager.get_threat_summary()
            self.threat_cards["critical"].set_value(summary.get("CRITICAL", 0))
            self.threat_cards["high"].set_value(summary.get("HIGH", 0))
            self.threat_cards["medium"].set_value(summary.get("MEDIUM", 0))
            self.threat_cards["total"].set_value(len(targets))

        if self.sensor_manager:
            for sid in ["RADAR_01", "RF_01", "ACOUSTIC_01", "LRF_01"]:
                sensor = self.sensor_manager.get_sensor(sid)
                if sensor:
                    self.sensor_status.update_sensor(sid, sensor)