"""Maintenance page for Tkinter"""
import tkinter as tk
from ui.styles.tk_theme import COLORS, FONTS


class MaintenancePage(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="MAINTENANCE", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        # System health
        health = tk.LabelFrame(self, text="SYSTEM HEALTH", bg=COLORS["bg_card"],
                                fg=COLORS["accent_cyan"], font=FONTS["bold"],
                                bd=1, relief=tk.SUNKEN)
        health.pack(fill=tk.X, padx=16, pady=4)

        items = [
            ("CPU Usage", "23%", COLORS["success"]),
            ("Memory", "1.2/8.0 GB", COLORS["success"]),
            ("Disk", "45.2/256 GB", COLORS["success"]),
            ("Temperature", "42°C", COLORS["success"]),
            ("Network I/O", "12.4 Mbps", COLORS["success"]),
            ("Database Size", "18.5 MB", COLORS["text_secondary"]),
        ]
        for label, value, color in items:
            f = tk.Frame(health, bg=COLORS["bg_card"])
            f.pack(fill=tk.X, padx=12, pady=2)
            tk.Label(f, text=label, fg=COLORS["text_secondary"], bg=COLORS["bg_card"],
                     font=FONTS["default"], width=20, anchor="w").pack(side=tk.LEFT)
            tk.Label(f, text=value, fg=color, bg=COLORS["bg_card"],
                     font=FONTS["bold"]).pack(side=tk.RIGHT)

        # Maintenance actions
        actions = tk.LabelFrame(self, text="MAINTENANCE ACTIONS", bg=COLORS["bg_card"],
                                 fg=COLORS["accent_cyan"], font=FONTS["bold"],
                                 bd=1, relief=tk.SUNKEN)
        actions.pack(fill=tk.X, padx=16, pady=8)

        action_buttons = [
            ("REBOOT SYSTEM", COLORS["accent_red"]),
            ("REPAIR DATABASE", COLORS["accent_orange"]),
            ("PURGE OLD DATA", COLORS["accent_yellow"]),
            ("UPDATE FIRMWARE", COLORS["accent_blue"]),
            ("RUN DIAGNOSTICS", COLORS["accent_green"]),
        ]
        for text, color in action_buttons:
            tk.Button(actions, text=text, bg=color,
                      fg="white" if color != COLORS["accent_green"] else "black",
                      font=FONTS["bold"], relief=tk.FLAT, padx=16, pady=6,
                      cursor="hand2").pack(pady=3, padx=12, anchor="w")

        # Log
        log_frame = tk.LabelFrame(self, text="RECENT EVENTS", bg=COLORS["bg_card"],
                                   fg=COLORS["accent_cyan"], font=FONTS["bold"],
                                   bd=1, relief=tk.SUNKEN)
        log_frame.pack(fill=tk.BOTH, expand=True, padx=16, pady=4)

        events = [
            ("12:38:16", "System started", COLORS["success"]),
            ("12:38:16", "Database connection established", COLORS["success"]),
            ("12:38:17", "All sensors initialized", COLORS["success"]),
            ("12:38:25", "Target detection active", COLORS["accent_cyan"]),
            ("12:40:00", "AI Engine processing", COLORS["accent_cyan"]),
        ]
        for time, desc, color in events:
            ef = tk.Frame(log_frame, bg=COLORS["bg_card"])
            ef.pack(fill=tk.X, padx=8, pady=1)
            tk.Label(ef, text=time, fg=color, bg=COLORS["bg_card"],
                     font=FONTS["small"], width=10, anchor="w").pack(side=tk.LEFT)
            tk.Label(ef, text=desc, fg=COLORS["text_secondary"], bg=COLORS["bg_card"],
                     font=FONTS["default"]).pack(side=tk.LEFT, padx=8)