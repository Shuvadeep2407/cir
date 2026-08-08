"""
MED-FLIGHT Counter-UAS Detection System
Alerts and notifications page
"""
from datetime import datetime
from PySide6.QtCore import Qt
from PySide6.QtGui import QColor, QFont
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QTableWidget, QTableWidgetItem,
                               QHeaderView, QComboBox, QFrame)
from utils.constants import COLORS, AlertSeverity


class AlertsPage(QWidget):
    """Alerts and notification management page"""
    def __init__(self, alert_manager=None, parent=None):
        super().__init__(parent)
        self.alert_manager = alert_manager
        self.setup_ui()
        if alert_manager:
            alert_manager.alert_created.connect(self._on_new_alert)

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        title = QLabel("ALERTS & NOTIFICATIONS")
        title.setObjectName("pageTitle")
        layout.addWidget(title)

        # Controls
        controls = QHBoxLayout()
        self.filter_combo = QComboBox()
        self.filter_combo.addItems(["ALL", "EMERGENCY", "CRITICAL", "WARNING", "INFO"])
        self.filter_combo.currentTextChanged.connect(self._filter)
        controls.addWidget(QLabel("Severity:"))
        controls.addWidget(self.filter_combo)

        controls.addStretch()

        self.ack_btn = QPushButton("ACKNOWLEDGE")
        self.ack_btn.setObjectName("primaryButton")
        self.ack_btn.clicked.connect(self._acknowledge_selected)
        controls.addWidget(self.ack_btn)

        self.clear_btn = QPushButton("CLEAR ALL")
        self.clear_btn.setObjectName("dangerButton")
        self.clear_btn.clicked.connect(self._clear)
        controls.addWidget(self.clear_btn)
        layout.addLayout(controls)

        # Table
        self.table = QTableWidget()
        self.table.setColumnCount(6)
        self.table.setHorizontalHeaderLabels(["TIME", "SEVERITY", "SOURCE", "TITLE", "MESSAGE", "STATUS"])
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setStretchLastSection(True)
        layout.addWidget(self.table)

    def _on_new_alert(self, alert):
        self._refresh()

    def _refresh(self):
        if not self.alert_manager:
            return
        alerts = self.alert_manager.alerts
        self._populate(alerts)

    def _filter(self):
        self._refresh()

    def _populate(self, alerts):
        severity_filter = self.filter_combo.currentText()
        if severity_filter != "ALL":
            alerts = [a for a in alerts if a.severity.label == severity_filter]

        self.table.setRowCount(len(alerts))
        for i, a in enumerate(alerts):
            self.table.setItem(i, 0, QTableWidgetItem(a.timestamp.strftime("%H:%M:%S")))
            
            sev_item = QTableWidgetItem(a.severity.label)
            sev_item.setForeground(QColor(a.severity.color))
            self.table.setItem(i, 1, sev_item)
            
            self.table.setItem(i, 2, QTableWidgetItem(a.source))
            self.table.setItem(i, 3, QTableWidgetItem(a.title))
            self.table.setItem(i, 4, QTableWidgetItem(a.message[:50]))
            
            status = "ACK'D" if a.acknowledged else "NEW"
            status_color = COLORS["text_muted"] if a.acknowledged else COLORS["accent_red"]
            st_item = QTableWidgetItem(status)
            st_item.setForeground(QColor(status_color))
            self.table.setItem(i, 5, st_item)

    def _acknowledge_selected(self):
        if not self.alert_manager:
            return
        row = self.table.currentRow()
        if row >= 0 and row < len(self.alert_manager.alerts):
            alert = self.alert_manager.alerts[row]
            self.alert_manager.acknowledge_alert(alert.alert_id)
            self._refresh()

    def _clear(self):
        if self.alert_manager:
            self.alert_manager.clear_all()
            self._refresh()