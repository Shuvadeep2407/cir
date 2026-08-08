"""
MED-FLIGHT Counter-UAS Detection System
Target list widget with sorting and filtering
"""
from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QColor, QFont
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QTableWidget,
                               QTableWidgetItem, QHeaderView, QLabel, QPushButton,
                               QLineEdit, QComboBox)
from utils.constants import COLORS, ThreatLevel


class TargetListWidget(QWidget):
    """Sortable, filterable list of detected targets"""
    target_selected = Signal(str)
    target_action = Signal(str, str)  # target_id, action

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(4)

        # Header
        header = QHBoxLayout()
        title = QLabel("TARGET LIST")
        title.setObjectName("sectionTitle")
        header.addWidget(title)
        header.addStretch()

        self.filter_btn = QPushButton("▼")
        self.filter_btn.setFixedWidth(30)
        header.addWidget(self.filter_btn)
        layout.addLayout(header)

        # Search bar
        search_layout = QHBoxLayout()
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("Search targets...")
        self.search_input.textChanged.connect(self._filter)
        search_layout.addWidget(self.search_input)

        self.threat_filter = QComboBox()
        self.threat_filter.addItems(["ALL", "CRITICAL", "HIGH", "MEDIUM", "LOW", "NONE"])
        self.threat_filter.currentTextChanged.connect(self._filter)
        search_layout.addWidget(self.threat_filter)
        layout.addLayout(search_layout)

        # Table
        self.table = QTableWidget()
        self.table.setColumnCount(7)
        self.table.setHorizontalHeaderLabels(["ID", "TYPE", "SPEED", "ALT", "HDG", "THREAT", "STATUS"])
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setSelectionMode(QTableWidget.SingleSelection)
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.table.setAlternatingRowColors(True)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)
        self.table.itemClicked.connect(self._on_select)
        layout.addWidget(self.table)

        # Action buttons
        action_layout = QHBoxLayout()
        self.engage_btn = QPushButton("ENGAGE")
        self.engage_btn.setObjectName("dangerButton")
        self.engage_btn.clicked.connect(lambda: self._action("engage"))
        action_layout.addWidget(self.engage_btn)

        self.track_btn = QPushButton("TRACK")
        self.track_btn.setObjectName("primaryButton")
        self.track_btn.clicked.connect(lambda: self._action("track"))
        action_layout.addWidget(self.track_btn)

        self.classify_btn = QPushButton("CLASSIFY")
        self.classify_btn.clicked.connect(lambda: self._action("classify"))
        action_layout.addWidget(self.classify_btn)
        layout.addLayout(action_layout)

    def update_targets(self, targets: list):
        self._all_targets = targets
        self._filter()

    def _filter(self):
        search = self.search_input.text().lower()
        threat_filter = self.threat_filter.currentText()

        filtered = self._all_targets if hasattr(self, '_all_targets') else []
        if search:
            filtered = [t for t in filtered if search in t.target_id.lower() or
                       search in t.classification.lower()]
        if threat_filter != "ALL":
            filtered = [t for t in filtered if t.threat_level.name == threat_filter]

        self._populate_table(filtered)

    def _populate_table(self, targets):
        self.table.setRowCount(len(targets))
        for i, t in enumerate(targets):
            self.table.setItem(i, 0, QTableWidgetItem(t.target_id))
            self.table.setItem(i, 1, QTableWidgetItem(t.classification[:12]))
            self.table.setItem(i, 2, QTableWidgetItem(f"{t.speed_ms:.1f}"))
            self.table.setItem(i, 3, QTableWidgetItem(f"{t.alt_m:.0f}"))
            self.table.setItem(i, 4, QTableWidgetItem(f"{t.heading_deg:.0f}°"))

            threat_item = QTableWidgetItem(t.threat_level.name)
            threat_item.setForeground(QColor(t.threat_level.color))
            self.table.setItem(i, 5, threat_item)

            status = "ENGAGED" if t.engaged else "TRACKING"
            self.table.setItem(i, 6, QTableWidgetItem(status))

    def _on_select(self, item):
        row = item.row()
        tid = self.table.item(row, 0).text()
        self.target_selected.emit(tid)

    def _action(self, action):
        row = self.table.currentRow()
        if row >= 0:
            tid = self.table.item(row, 0).text()
            self.target_action.emit(tid, action)