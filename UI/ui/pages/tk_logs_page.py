"""Logs page for Tkinter"""
import tkinter as tk
from tkinter import ttk
from ui.styles.tk_theme import COLORS, FONTS


class LogsPage(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="SYSTEM LOGS", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        controls = tk.Frame(self, bg=COLORS["bg_dark"])
        controls.pack(fill=tk.X, padx=16, pady=4)
        tk.Button(controls, text="REFRESH", bg=COLORS["accent_blue"], fg="white",
                  font=FONTS["bold"], relief=tk.FLAT, padx=12, cursor="hand2").pack(side=tk.RIGHT)
        tk.Button(controls, text="EXPORT", bg=COLORS["bg_tertiary"], fg=COLORS["text_primary"],
                  font=FONTS["bold"], relief=tk.FLAT, padx=12, cursor="hand2").pack(side=tk.RIGHT, padx=4)

        # Log text area
        self.log_text = tk.Text(self, bg=COLORS["bg_primary"], fg=COLORS["text_secondary"],
                                 font=FONTS["mono"], bd=1, relief=tk.SUNKEN,
                                 wrap=tk.NONE, padx=8, pady=8)
        self.log_text.pack(fill=tk.BOTH, expand=True, padx=16, pady=8)

        # Sample log entries
        sample_logs = [
            "[2026-06-24 12:38:16] INFO  | System initialized",
            "[2026-06-24 12:38:16] INFO  | Database schema created/verified",
            "[2026-06-24 12:38:16] INFO  | Created sensor: AN/MPQ-64 Radar (RADAR_01)",
            "[2026-06-24 12:38:16] INFO  | Created sensor: RF-3000 Scanner (RF_01)",
            "[2026-06-24 12:38:16] INFO  | New target detected: UAS-0000 (DJI_MAVIC_3)",
            "[2026-06-24 12:38:16] INFO  | New target detected: UAS-0001 (DJI_PHANTOM_4)",
            "[2026-06-24 12:38:16] WARN  | Sensor calibration recommended",
            "[2026-06-24 12:38:16] INFO  | MED-FLIGHT C-UAS Detection System v2.4.0 initialized",
            "[2026-06-24 12:38:25] WARN  | ALERT [INFO] SYSTEM: Target Detected",
            "[2026-06-24 12:38:25] INFO  | New target: UAS-0003 (AUTEL_ROBOTICS)",
        ]
        for line in sample_logs:
            self.log_text.insert(tk.END, line + "\n")
        self.log_text.configure(state=tk.DISABLED)

        # Scrollbar
        scroll = tk.Scrollbar(self.log_text, command=self.log_text.yview)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.configure(yscrollcommand=scroll.set)