"""Dashboard page for Tkinter"""
import tkinter as tk
from ui.styles.tk_theme import COLORS, FONTS
from ui.components.tk_tactical_map import TacticalMapWidget
from ui.components.tk_video_panel import VideoPanel
from ui.components.tk_sensor_status import SensorStatusPanel
from ui.components.tk_target_list import TargetListWidget


class ThreatSummaryCard(tk.Frame):
    def __init__(self, parent, title, initial_value, color, subtitle="", **kw):
        super().__init__(parent, bg=COLORS["bg_card"], bd=1, relief=tk.SUNKEN, **kw)
        self.color = color
        self.configure(highlightbackground=color, highlightthickness=2)
        tk.Label(self, text=title, fg=COLORS["text_muted"], bg=COLORS["bg_card"],
                 font=FONTS["small"]).pack(anchor="w", padx=12, pady=(8, 0))
        self.value_lbl = tk.Label(self, text=str(initial_value), fg=color,
                                  bg=COLORS["bg_card"], font=FONTS["title"])
        self.value_lbl.pack(anchor="w", padx=12)
        if subtitle:
            tk.Label(self, text=subtitle, fg=COLORS["text_secondary"],
                     bg=COLORS["bg_card"], font=FONTS["small"]).pack(anchor="w", padx=12, pady=(0, 8))

    def set_value(self, value):
        self.value_lbl.configure(text=str(value))


class DashboardPage(tk.Frame):
    def __init__(self, parent, target_manager=None, sensor_manager=None, alert_manager=None, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.target_manager = target_manager
        self.sensor_manager = sensor_manager
        self.alert_manager = alert_manager
        self.setup_ui()
        self._update_timer = None
        self._start_updates()

    def setup_ui(self):
        # Status bar at top
        status_frame = tk.Frame(self, bg=COLORS["bg_secondary"], bd=1, relief=tk.SUNKEN)
        status_frame.pack(fill=tk.X, padx=8, pady=4)
        items = [
            ("SYS", "ACTIVE", COLORS["success"]),
            ("SENSORS", "9/9", COLORS["success"]),
            ("NET", "LINK", COLORS["success"]),
            ("GPS", "LOCK", COLORS["success"]),
            ("REC", "STANDBY", COLORS["warning"]),
            ("UPTIME", "02:34:12", COLORS["text_secondary"]),
        ]
        for label, value, color in items:
            lbl = tk.Label(status_frame, text=f"{label} {value}",
                           fg=COLORS["text_secondary"], bg=COLORS["bg_secondary"],
                           font=FONTS["small"])
            lbl.pack(side=tk.LEFT, padx=8, pady=4)
            tk.Frame(status_frame, width=1, bg=COLORS["border"]).pack(side=tk.LEFT, fill=tk.Y)

        # Threat summary cards
        cards_frame = tk.Frame(self, bg=COLORS["bg_dark"])
        cards_frame.pack(fill=tk.X, padx=8, pady=4)
        self.threat_cards = {
            "critical": ThreatSummaryCard(cards_frame, "CRITICAL", "0", COLORS["accent_red"], "Immediate action"),
            "high": ThreatSummaryCard(cards_frame, "HIGH", "0", COLORS["accent_orange"], "Priority tracking"),
            "medium": ThreatSummaryCard(cards_frame, "MEDIUM", "0", COLORS["accent_yellow"], "Monitor"),
            "total": ThreatSummaryCard(cards_frame, "TOTAL", "0", COLORS["accent_cyan"], "Active tracks"),
        }
        for card in self.threat_cards.values():
            card.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=3)

        # Main content: Radar (left) + Cam/Sensors (right)
        content = tk.Frame(self, bg=COLORS["bg_dark"])
        content.pack(fill=tk.BOTH, expand=True, padx=8, pady=4)

        # Left: Tactical map
        left = tk.Frame(content, bg=COLORS["bg_dark"])
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        map_header = tk.Label(left, text="RADAR VIEW", fg=COLORS["accent_cyan"],
                              bg=COLORS["bg_dark"], font=FONTS["section"])
        map_header.pack(fill=tk.X)
        self.tactical_map = TacticalMapWidget(left)
        self.tactical_map.pack(fill=tk.BOTH, expand=True, pady=4)
        self.tactical_map.animate()

        # Right: Video + Sensors
        right = tk.Frame(content, bg=COLORS["bg_dark"], width=500)
        right.pack(side=tk.RIGHT, fill=tk.BOTH)
        right.pack_propagate(False)

        self.video_panel = VideoPanel(right)
        self.video_panel.pack(fill=tk.BOTH, expand=True)

        self.sensor_status = SensorStatusPanel(right)
        self.sensor_status.pack(fill=tk.BOTH, pady=(4, 0))

    def _start_updates(self):
        def update():
            if self.target_manager:
                targets = self.target_manager.get_all_targets()
                self.tactical_map.set_targets(targets)
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

            self._update_timer = self.after(500, update)
        update()

    def destroy(self):
        if self._update_timer:
            self.after_cancel(self._update_timer)
        self.video_panel.stop()
        super().destroy()