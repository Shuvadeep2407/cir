"""
 Counter-UAS Detection System
Tkinter Application Entry Point
"""
import sys
import os
import tkinter as tk
from tkinter import ttk, font
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))

from utils.logger import log
from utils.constants import APP_NAME, APP_VERSION
from database.db_manager import DatabaseManager
from core.sensor_manager import SensorManager
from core.target_manager import TargetManager
from core.alert_manager import AlertManager
from core.sensor_fusion import SensorFusionEngine
from core.ai_engine import AIEngine
from core.mission_recorder import MissionRecorder
from core.playback_engine import PlaybackEngine
from network.stream_handler import StreamHandler
from ui.styles.tk_theme import COLORS, FONTS


class NotificationBadge(tk.Canvas):
    def __init__(self, parent, **kw):
        super().__init__(parent, width=20, height=16, highlightthickness=0, bg=COLORS["bg_primary"], **kw)
        self._count = 0
        self._badge_id = None

    def update_count(self, count: int):
        self.delete("all")
        self._count = count
        if count > 0:
            text = str(min(count, 99))
            self.create_oval(2, 2, 18, 14, fill=COLORS["accent_red"], outline="")
            self.create_text(10, 8, text=text, fill="white",
                             font=("Consolas", 7, "bold"))


class SidebarButton(tk.Frame):
    def __init__(self, parent, text, icon, index, command, **kw):
        super().__init__(parent, bg=COLORS["bg_primary"], **kw)
        self.index = index
        self._selected = False
        self._text = text
        self._icon = icon

        self.indicator = tk.Frame(self, width=3, bg=COLORS["bg_primary"])
        self.indicator.pack(side=tk.LEFT, fill=tk.Y)

        self.lbl = tk.Label(self, text=f"  {icon} {text}", anchor="w",
                            bg=COLORS["bg_primary"], fg=COLORS["text_secondary"],
                            font=FONTS["default"], padx=16, pady=10)
        self.lbl.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.badge_frame = tk.Frame(self, bg=COLORS["bg_primary"])
        self.badge_frame.pack(side=tk.RIGHT, padx=(0, 8))

        self.lbl.bind("<Button-1>", lambda e: command(index))
        self.bind("<Button-1>", lambda e: command(index))

    def set_selected(self, selected: bool):
        self._selected = selected
        if selected:
            self.indicator.configure(bg=COLORS["accent_cyan"])
            self.lbl.configure(bg=COLORS["bg_tertiary"], fg=COLORS["accent_cyan"])
        else:
            self.indicator.configure(bg=COLORS["bg_primary"])
            self.lbl.configure(bg=COLORS["bg_primary"], fg=COLORS["text_secondary"])


class LoginDialog(tk.Toplevel):
    def __init__(self, parent, on_login):
        super().__init__(parent)
        self.on_login = on_login
        self.overrideredirect(True)
        self.configure(bg=COLORS["bg_dark"], highlightbackground=COLORS["border_active"],
                       highlightthickness=2)
        self.geometry("380x320")
        self.resizable(False, False)

        # Center on parent
        self.update_idletasks()
        pw, ph = parent.winfo_width(), parent.winfo_height()
        px, py = parent.winfo_x(), parent.winfo_y()
        x = px + (pw - 380) // 2
        y = py + (ph - 320) // 2
        self.geometry(f"+{x}+{y}")

        frame = tk.Frame(self, bg=COLORS["bg_dark"])
        frame.pack(expand=True, fill=tk.BOTH, padx=32, pady=32)

        tk.Label(frame, text="", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["huge"]).pack()

        tk.Label(frame, text="C-UAS Detection System v2.4.0",
                 fg=COLORS["text_muted"], bg=COLORS["bg_dark"],
                 font=FONTS["default"]).pack(pady=(4, 20))

        self.status_lbl = tk.Label(frame, text="", fg=COLORS["accent_red"],
                                   bg=COLORS["bg_dark"], font=FONTS["small"])
        self.status_lbl.pack()

        self.login_btn = tk.Button(frame, text="INITIALIZE SYSTEM",
                                   command=self._do_login,
                                   bg=COLORS["accent_blue"], fg="white",
                                   font=FONTS["bold"], relief=tk.FLAT,
                                   padx=20, pady=10, cursor="hand2",
                                   activebackground="#0055dd", activeforeground="white")
        self.login_btn.pack(pady=16)

        tk.Label(frame, text=f"Build {APP_VERSION}", fg=COLORS["text_muted"],
                 bg=COLORS["bg_dark"], font=FONTS["small"]).pack()

    def _do_login(self):
        self.login_btn.configure(text="INITIALIZING...", state=tk.DISABLED)
        self.after(100, self.on_login)


class MainApplication(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(f"{APP_NAME} v{APP_VERSION}")
        self.geometry("1600x900")
        self.configure(bg=COLORS["bg_dark"])
        self.minsize(1200, 700)

        # Initialize database
        self.db = DatabaseManager()

        # Initialize core systems (pass None parent since Tkinter isn't a QObject)
        self.sensor_manager = SensorManager(None)
        self.target_manager = TargetManager(None)
        self.alert_manager = AlertManager(None)
        self.fusion_engine = SensorFusionEngine(None)
        self.ai_engine = AIEngine(None)
        self.mission_recorder = MissionRecorder(None)
        self.playback_engine = PlaybackEngine(None)
        self.stream_handler = StreamHandler(None)

        # Wire up connections (simplified for Tkinter)
        self.target_manager.target_added_callback = self._on_target_added
        self.target_manager.target_updated_callback = self._on_target_updated
        self.alert_manager.alert_created_callback = self._on_new_alert

        self._setup_ui()
        self._show_login()

        log.info(f"{APP_NAME} v{APP_VERSION} initialized")

    def _show_login(self):
        self.withdraw()
        self.login = LoginDialog(self, self._initialize_system)
        self.login.grab_set()

    def _setup_ui(self):
        # Main container
        self.main_frame = tk.Frame(self, bg=COLORS["bg_dark"])
        self.main_frame.pack(fill=tk.BOTH, expand=True)

        # Sidebar
        self.sidebar = tk.Frame(self.main_frame, width=220, bg=COLORS["bg_primary"])
        self.sidebar.pack(side=tk.LEFT, fill=tk.Y)

        # Separator line
        sep = tk.Frame(self.sidebar, height=1, bg=COLORS["border"])
        sep.pack(fill=tk.X)

        # Logo
        logo = tk.Label(self.sidebar, text="\nC-UAS SYSTEM",
                        fg=COLORS["accent_cyan"], bg=COLORS["bg_primary"],
                        font=FONTS["bold"], pady=16)
        logo.pack(fill=tk.X)

        # Nav buttons
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
            btn_frame = SidebarButton(self.sidebar, text, icon, idx, self._switch_page)
            btn_frame.pack(fill=tk.X)
            self.nav_buttons.append(btn_frame)
            if text == "ALERTS":
                self.alert_badge = NotificationBadge(btn_frame.badge_frame)
                self.alert_badge.pack()

        # System status at bottom
        self.sys_label = tk.Label(self.sidebar, text="SYS: ACTIVE",
                                  fg=COLORS["text_muted"], bg=COLORS["bg_primary"],
                                  font=FONTS["small"], pady=8)
        self.sys_label.pack(side=tk.BOTTOM, fill=tk.X)

        # Separator
        sep2 = tk.Frame(self.sidebar, height=1, bg=COLORS["border"])
        sep2.pack(side=tk.BOTTOM, fill=tk.X)

        # Content area
        self.content_frame = tk.Frame(self.main_frame, bg=COLORS["bg_dark"])
        self.content_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Stack of pages
        self.pages = []
        self._current_page = 0

        # Import and create pages
        from ui.pages.tk_dashboard_page import DashboardPage
        from ui.pages.tk_alerts_page import AlertsPage
        from ui.pages.tk_settings_page import SettingsPage
        from ui.pages.tk_calibration_page import CalibrationPage
        from ui.pages.tk_logs_page import LogsPage
        from ui.pages.tk_users_page import UsersPage
        from ui.pages.tk_playback_page import PlaybackPage
        from ui.pages.tk_maintenance_page import MaintenancePage

        # Dashboard (index 0)
        d = DashboardPage(self.content_frame, self.target_manager, self.sensor_manager, self.alert_manager)
        self.pages.append(d)

        # Targets (index 1)
        from ui.components.tk_target_list import TargetListWidget
        t = TargetListWidget(self.content_frame)
        self.target_list = t
        self.pages.append(t)

        # Alerts (index 2)
        a = AlertsPage(self.content_frame, self.alert_manager)
        self.pages.append(a)

        # Logs (index 3)
        l = LogsPage(self.content_frame)
        self.pages.append(l)

        # Users (index 4)
        u = UsersPage(self.content_frame)
        self.pages.append(u)

        # Settings (index 5)
        s = SettingsPage(self.content_frame)
        self.pages.append(s)

        # Calibration (index 6)
        c = CalibrationPage(self.content_frame)
        self.pages.append(c)

        # Playback (index 7)
        p = PlaybackPage(self.content_frame)
        self.pages.append(p)

        # Maintenance (index 8)
        m = MaintenancePage(self.content_frame)
        self.pages.append(m)

        # Show first page
        self._switch_page(0)

    def _switch_page(self, index):
        if index < 0 or index >= len(self.pages):
            return
        # Hide current
        if self._current_page < len(self.pages):
            self.pages[self._current_page].pack_forget()
            self.nav_buttons[self._current_page].set_selected(False)

        # Show new
        self._current_page = index
        self.pages[index].pack(fill=tk.BOTH, expand=True)
        self.nav_buttons[index].set_selected(True)

    def _initialize_system(self):
        self.login.destroy()
        self.deiconify()

        # Start sensors
        self.sensor_manager.start_all()
        self.stream_handler.start()

        # Initial alerts
        self.alert_manager.create_system_alert("System Initialized", "All subsystems online")
        self.alert_manager.create_warning("Sensor calibration recommended",
                                          "LRF-2200 calibration due in 12 hours")
        log.info("System fully initialized")

    def _on_new_alert(self, alert):
        if hasattr(self, 'alert_badge') and self.alert_manager:
            unread = sum(1 for a in self.alert_manager.alerts if not a.acknowledged)
            self.alert_badge.update_count(unread)
        # Refresh alerts page if visible
        if self._current_page == 2:
            self.pages[2]._refresh()

    def _on_target_added(self, target):
        from utils.constants import ThreatLevel
        tag = f"New target: {target.target_id} ({target.classification})"
        self.alert_manager.create_system_alert("Target Detected", tag)
        self.ai_engine.classify_target(target)
        if hasattr(self, 'target_list'):
            self.target_list.update_targets(self.target_manager.get_all_targets())
        log.info(tag)

    def _on_target_updated(self, target):
        if hasattr(self, 'target_list'):
            self.target_list.update_targets(self.target_manager.get_all_targets())

    def destroy(self):
        log.info("Shutting down...")
        self.sensor_manager.stop_all()
        self.stream_handler.stop_all()
        self.db.close()
        super().destroy()


def main():
    for d in ["data", "logs", "recordings"]:
        Path(d).mkdir(exist_ok=True)

    app = MainApplication()
    log.info(f"{APP_NAME} v{APP_VERSION} started")
    app.mainloop()


if __name__ == "__main__":
    main()