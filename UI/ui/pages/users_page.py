"""User management page"""
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                               QPushButton, QTableWidget, QTableWidgetItem,
                               QHeaderView, QDialog, QFormLayout, QLineEdit,
                               QComboBox, QDialogButtonBox, QCheckBox)
from utils.constants import COLORS


class UserDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Add/Edit User")
        self.setFixedSize(400, 350)
        layout = QFormLayout(self)
        self.username = QLineEdit()
        self.password = QLineEdit()
        self.password.setEchoMode(QLineEdit.Password)
        self.full_name = QLineEdit()
        self.role = QComboBox()
        self.role.addItems(["ADMIN", "OPERATOR", "ANALYST", "MAINTENANCE"])
        self.email = QLineEdit()
        layout.addRow("Username:", self.username)
        layout.addRow("Password:", self.password)
        layout.addRow("Full Name:", self.full_name)
        layout.addRow("Role:", self.role)
        layout.addRow("Email:", self.email)
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)


class UsersPage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setup_ui()

    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)
        title = QLabel("USER MANAGEMENT")
        title.setObjectName("pageTitle")
        layout.addWidget(title)
        controls = QHBoxLayout()
        self.add_btn = QPushButton("+ ADD USER")
        self.add_btn.setObjectName("successButton")
        self.add_btn.clicked.connect(self._add_user)
        controls.addWidget(self.add_btn)
        self.edit_btn = QPushButton("EDIT")
        self.edit_btn.setObjectName("primaryButton")
        controls.addWidget(self.edit_btn)
        self.deactivate_btn = QPushButton("DEACTIVATE")
        self.deactivate_btn.setObjectName("dangerButton")
        controls.addWidget(self.deactivate_btn)
        controls.addStretch()
        layout.addLayout(controls)
        self.table = QTableWidget()
        self.table.setColumnCount(7)
        self.table.setHorizontalHeaderLabels(["ID", "USERNAME", "FULL NAME", "ROLE", "EMAIL", "ACTIVE", "LAST LOGIN"])
        self.table.setSelectionBehavior(QTableWidget.SelectRows)
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setStretchLastSection(True)
        layout.addWidget(self.table)
        self._add_sample_users()

    def _add_sample_users(self):
        users = [
            ("ADMIN01", "admin", "System Administrator", "ADMIN", "admin@medflight.com", "Yes", "2026-06-24 08:00"),
            ("OP001", "operator", "Mission Operator", "OPERATOR", "ops@medflight.com", "Yes", "2026-06-24 07:30"),
            ("AN001", "analyst", "Intel Analyst", "ANALYST", "analyst@medflight.com", "Yes", "2026-06-23 22:00"),
            ("MT001", "tech", "Maintenance Tech", "MAINTENANCE", "tech@medflight.com", "Yes", "2026-06-22 14:00"),
        ]
        self.table.setRowCount(len(users))
        for i, u in enumerate(users):
            for j, v in enumerate(u):
                self.table.setItem(i, j, QTableWidgetItem(str(v)))

    def _add_user(self):
        dialog = UserDialog(self)
        if dialog.exec() == QDialog.Accepted:
            row = self.table.rowCount()
            self.table.insertRow(row)
            self.table.setItem(row, 0, QTableWidgetItem(f"USR{row+1:04d}"))
            self.table.setItem(row, 1, QTableWidgetItem(dialog.username.text()))
            self.table.setItem(row, 2, QTableWidgetItem(dialog.full_name.text()))
            self.table.setItem(row, 3, QTableWidgetItem(dialog.role.currentText()))
            self.table.setItem(row, 4, QTableWidgetItem(dialog.email.text()))
            self.table.setItem(row, 5, QTableWidgetItem("Yes"))
            self.table.setItem(row, 6, QTableWidgetItem("Never"))