"""
MED-FLIGHT Counter-UAS Detection System
Main application entry point
"""
import sys
import os
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))

from PySide6.QtCore import Qt, QTimer, QPropertyAnimation, QEasingCurve, QPoint, QSize
from PySide6.QtGui import QIcon, QFont
from PySide6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                               QHBoxLayout, QPushButton, QStackedWidget, QLabel,
                               QFrame, QStatusBar, QMessageBox, QScrollArea)

from utils.logger import log
from utils.constants import APP_NAME, APP_VERSION, COLORS
from database.db_manager import DatabaseManager
from ui.styles.theme import ThemeManager
from ui.pages.dashboard_page import DashboardPage
from ui.pages.alerts_page import AlertsPage
from ui.pages.logs_page import LogsPage
from ui.pages.users_page import UsersPage
from ui.pages.settings_page import SettingsPage
from ui.pages.calibration_page import CalibrationPage
from ui.pages.playback_page import PlaybackPage
from ui.pages.maintenance_page import MaintenancePage
from core.sensor_manager import SensorManager
from core.target_manager import TargetManager
from core.alert_manager import AlertManager
from core.sensor_fusion import SensorFusionEngine
from core.ai_engine import AIEngine
from core.mission_recorder import MissionRecorder
from core.playback_engine import PlaybackEngine
from network.stream_handler import StreamHandler


class LoginDialog(QWidget):
    """Simple login overlay"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setStyleSheet(f"""
            background-color: {COLORS["bg_dark"]};
            border: 2px solid {COLORS["border_active"]};
            border-radius: 12px;
        """)
        self.setFixedSize(380, 320)
        layout = QVBoxLayout(self)
        layout.setSpacing(16)
        layout.setContentsMargins(32, 32, 32, 32)

        title = QLabel("MED-FLIGHT")
        title.setStyleSheet(f"color: {COLORS['accent_cyan']}; font-size: 28px; font-weight: bold;")
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        subtitle = QLabel("C-UAS Detection System v2.4.0")
        subtitle.setStyleSheet(f"color: {COLORS['text_muted']}; font-size: 12px;")
        subtitle.setAlignment(Qt.AlignCenter)
        layout.addWidget(subtitle)

        layout.addSpacing(20)

        self.status_lbl = QLabel("")
        self.status_lbl.setStyleSheet(f"color: {COLORS['accent_red']}; font-size: 11px;")
        self.status_lbl.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.status_lbl)

        self.login_btn = QPushButton("INITIALIZE SYSTEM")
        self.login_btn.setObjectName("primaryButton")
        self.login_btn.setMinimumHeight(40)
        self.login_btn.setStyleSheet(f"""
            QPushButton {{
                background-color: {COLORS['accent_blue']};
                border: none; border-radius: 6px;
                font-size: 14px; font-weight: bold; color: white;
                padding: 10px;
            }}
            QPushButton:hover {{ background-color: #0055dd; }}
        """)
        layout.addWidget(self.login_btn)

        version = QLabel(f"Build {APP_VERSION}")
        version.setStyleSheet(f"color: {COLORS['text_muted']}; font-size: 9px;")
        version.setAlignment(Qt.AlignCenter)
        layout.addWidget(version)


class NotificationBadge(QLabel):
    """Unread notification count badge"""
    def __init__(self, parent=None):
        super().__init__("0", parent)
        self.setFixedSize(20, 16)
        self.setAlignment(Qt.AlignCenter)
        self.setStyleSheet(f"""
            QLabel {{
                background-color: {COLORS['accent_red']};
                color: white;
                font-size: 9px;
                font-weight: bold;
                border-radius: 8px;
                padding: 0 4px;
            }}
        """)
        self.hide()

    def update_count(self, count: int):
        if count > 0:
            self.setText(str(min(count, 99)))
            self.show()
        else:
            self.hide()


class SidebarButton(QPushButton):
    """Styled sidebar navigation button"""
    def __init__(self, text, icon_char="▶", parent=None):
        super().__init__(parent)
        self.setText(f"  {icon_char} {text}")
        self.setObjectName("sidebarButton")
        self.setCheckable(True)
        self.setMinimumHeight(36)

    def set_badge(self, count: int):
        pass  # Handled by MainWindow


class MainWindow(QMainWindow):
    """Main application window"""
    def __init__(self):
        super().__init__()
        self.setWindowTitle(f"{APP_NAME} v{APP_VERSION}")
        self.setMinimumSize(1600, 900)
        self.setStyleSheet(f"background-color: {COLORS['bg_dark']};")

        # Initialize database
        self.db = DatabaseManager()

        # Initialize core systems
        self.sensor_manager = SensorManager(self)
        self.target_manager = TargetManager(self)
        self.alert_manager = AlertManager(self)
        self.fusion_engine = SensorFusionEngine(self)
        self.ai_engine = AIEngine(self)
        self.mission_recorder = MissionRecorder(self)
        self.playback_engine = PlaybackEngine(self)
        self.stream_handler = StreamHandler(self)

        # Wire up connections
        self.sensor_manager.new_reading.connect(self.fusion_engine.process_reading)
        self.target_manager.target_added.connect(self._on_target_added)
        self.target_manager.target_updated.connect(self._on_target_updated)
        self.alert_manager.alert_created.connect(self._on_new_alert)

        # Show login first
        self.login_overlay = LoginDialog(self)
        self.login_overlay.login_btn.clicked.connect(self._initialize_system)

        # Setup main UI
        self._setup_ui()
        self.login_overlay.show()

        log.info(f"{APP_NAME} v{APP_VERSION} initialized")

    def _setup_ui(self):
        """Build the main user interface"""
        central = QWidget()
        self.setCentralWidget(central)
        main_layout = QHBoxLayout(central)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        # Sidebar
        self.sidebar = QFrame()
        self.sidebar.setFixedWidth(220)
        self.sidebar.setStyleSheet(f"""
            QFrame {{
                background-color: {COLORS["bg_primary"]};
                border-right: 1px solid {COLORS["border"]};
            }}
        """)
        sidebar_layout = QVBoxLayout(self.sidebar)
        sidebar_layout.setContentsMargins(0, 0, 0, 0)
        sidebar_layout.setSpacing(0)

        # Logo area
        logo = QLabel("MED-FLIGHT\nC-UAS SYSTEM")
        logo.setStyleSheet(f"color: {COLORS['accent_cyan']}; font-size: 14px; font-weight: bold; padding: 16px;")
        logo.setAlignment(Qt.AlignCenter)
        logo.setFixedHeight(60)
        sidebar_layout.addWidget(logo)

        # Separator
        sep = QFrame()
        sep.setFrameShape(QFrame.HLine)
        sep.setStyleSheet(f"background-color: {COLORS['border']}; max-height: 1px;")
        sidebar_layout.addWidget(sep)

        # Navigation buttons
        nav_items = [
            ("DASHBOARD", "◈", 0),
            ("TARGETS", "◆", 1),
            ("ALERTS", "⚠", 2),
            ("LOGS", "📋", 3),
            ("USERS", "👤", 4),
            ("SETTINGS", "⚙", 5),
            ("CALIBRATION", "🎯", 6),
            ("PLAYBACK", "▶", 7),
            ("MAINTENANCE", "🔧", 8),
        ]
        self.nav_buttons = []
        for text, icon, idx in nav_items:
            btn = SidebarButton(text, icon)
            btn.clicked.connect(lambda checked, i=idx: self._switch_page(i))
            btn_layout = QHBoxLayout()
            btn_layout.setContentsMargins(0, 0, 8, 0)
            btn_layout.addWidget(btn)
            if text == "ALERTS":
                self.alert_badge = NotificationBadge()
                badge_container = QWidget()
                badge_container.setFixedWidth(30)
                bc_layout = QHBoxLayout(badge_container)
                bc_layout.setContentsMargins(0, 0, 0, 0)
                bc_layout.addWidget(self.alert_badge)
                btn_layout.addWidget(badge_container)
            sidebar_layout.addLayout(btn_layout)
            self.nav_buttons.append(btn)

        sidebar_layout.addStretch()

        # System info at bottom
        sys_info = QLabel(f"SYS: ACTIVE")
        sys_info.setStyleSheet(f"color: {COLORS['text_muted']}; font-size: 9px; padding: 8px;")
        sys_info.setAlignment(Qt.AlignCenter)
        sidebar_layout.addWidget(sys_info)

        main_layout.addWidget(self.sidebar)

        # Content area
        self.content_stack = QStackedWidget()

        # Create pages
        self.dashboard = DashboardPage(self.target_manager, self.sensor_manager, self.alert_manager)
        self.content_stack.addWidget(self.dashboard)  # 0

        target_page = QWidget()  # Placeholder for dedicated target page
        target_layout = QVBoxLayout(target_page)
        from ui.components.target_list import TargetListWidget
        self.target_list = TargetListWidget()
        target_layout.addWidget(self.target_list)
        self.content_stack.addWidget(target_page)  # 1

        self.alerts_page = AlertsPage(self.alert_manager)
        self.alerts_page.setObjectName("page_alerts")
        self.content_stack.addWidget(self.alerts_page)  # 2

        self.logs_page = LogsPage()
        self.content_stack.addWidget(self.logs_page)  # 3

        self.users_page = UsersPage()
        self.content_stack.addWidget(self.users_page)  # 4

        self.settings_page = SettingsPage()
        self.content_stack.addWidget(self.settings_page)  # 5

        self.calibration_page = CalibrationPage()
        self.content_stack.addWidget(self.calibration_page)  # 6

        self.playback_page = PlaybackPage(self.playback_engine)
        self.content_stack.addWidget(self.playback_page)  # 7

        self.maintenance_page = MaintenancePage()
        self.content_stack.addWidget(self.maintenance_page)  # 8

        main_layout.addWidget(self.content_stack, 1)

        # Select first page
        self._switch_page(0)

    def _switch_page(self, index):
        """Switch to the given page index"""
        # Animated page transition
        current = self.content_stack.currentWidget()
        next_widget = self.content_stack.widget(index)
        if current and next_widget and current != next_widget:
            # Slide animation
            next_widget.setGraphicsEffect(None)
            self._slide_to_widget(current, next_widget, index)

        self.content_stack.setCurrentIndex(index)
        for i, btn in enumerate(self.nav_buttons):
            btn.setChecked(i == index)

    def _initialize_system(self):
        """Initialize system after login"""
        self.login_overlay.login_btn.setText("INITIALIZING...")
        self.login_overlay.login_btn.setEnabled(False)
        QApplication.processEvents()

        # Start sensors
        self.sensor_manager.start_all()
        self.stream_handler.start()

        # Generate some initial alerts
        self.alert_manager.create_system_alert("System Initialized", "All subsystems online")
        self.alert_manager.create_warning("Sensor calibration recommended",
                                          "LRF-2200 calibration due in 12 hours")

        self.login_overlay.hide()
        self.show()

        # Animate sidebar buttons in
        for i, btn in enumerate(self.nav_buttons):
            anim = QPropertyAnimation(btn, b"pos")
            anim.setDuration(300)
            anim.setStartValue(QPoint(-220, btn.y()))
            anim.setEndValue(QPoint(0, btn.y()))
            anim.setEasingCurve(QEasingCurve.OutBack)
            anim.setStartTime(i * 50)
            anim.start()

        log.info("System fully initialized")

    def _on_new_alert(self, alert):
        """Update notification badge when new alert arrives"""
        if self.alert_manager:
            unread = sum(1 for a in self.alert_manager.alerts if not a.acknowledged)
            self.alert_badge.update_count(unread)
            # Pulse effect on alerts button
            btn = self.nav_buttons[2]
            btn.setStyleSheet(f"""
                QPushButton#sidebarButton {{
                    background-color: {COLORS['bg_secondary']};
                    border: none; border-radius: 0;
                    text-align: left; padding: 10px 16px; font-size: 12px;
                    color: {COLORS['accent_red']};
                    border-left: 3px solid {COLORS['accent_red']};
                    font-weight: bold;
                }}
            """)
            QTimer.singleShot(2000, lambda: self._reset_alert_btn_style())

    def _reset_alert_btn_style(self):
        btn = self.nav_buttons[2]
        btn.setStyleSheet("")

    def _on_target_added(self, target):
        tag = f"New target: {target.target_id} ({target.classification})"
        self.alert_manager.create_system_alert("Target Detected", tag)
        self.ai_engine.classify_target(target)
        self.target_list.update_targets(self.target_manager.get_all_targets())
        log.info(tag)

    def _on_target_updated(self, target):
        self.target_list.update_targets(self.target_manager.get_all_targets())

    def _slide_to_widget(self, current, next_widget, index):
        """Animated slide transition between pages"""
        direction = 1 if index > self.content_stack.currentIndex() else -1
        w = self.content_stack.width()

        # Position next widget off-screen
        next_widget.raise_()
        next_widget.show()
        start_x = direction * w
        next_widget.move(start_x, 0)
        next_widget.setFixedSize(self.content_stack.size())

        # Animate both
        self._anim_curr = QPropertyAnimation(current, b"pos")
        self._anim_curr.setDuration(250)
        self._anim_curr.setStartValue(QPoint(0, 0))
        self._anim_curr.setEndValue(QPoint(-direction * w, 0))
        self._anim_curr.setEasingCurve(QEasingCurve.OutCubic)

        self._anim_next = QPropertyAnimation(next_widget, b"pos")
        self._anim_next.setDuration(250)
        self._anim_next.setStartValue(QPoint(start_x, 0))
        self._anim_next.setEndValue(QPoint(0, 0))
        self._anim_next.setEasingCurve(QEasingCurve.OutCubic)

        self._anim_curr.start()
        self._anim_next.start()

    def closeEvent(self, event):
        """Clean shutdown"""
        log.info("Shutting down...")
        self.sensor_manager.stop_all()
        self.stream_handler.stop_all()
        self.db.close()
        event.accept()


def main():
    """Application entry point"""
    # Create data directories
    for d in ["data", "logs", "recordings"]:
        Path(d).mkdir(exist_ok=True)

    app = QApplication(sys.argv)
    app.setApplicationName("MED-FLIGHT C-UAS")
    app.setOrganizationName("MED-FLIGHT Defense Systems")

    # Apply theme
    ThemeManager.apply(app)

    # Create and show main window
    window = MainWindow()
    log.info(f"{APP_NAME} v{APP_VERSION} started")

    sys.exit(app.exec())


if __name__ == "__main__":
    main()