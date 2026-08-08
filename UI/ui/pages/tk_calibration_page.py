"""Calibration page for Tkinter"""
import tkinter as tk
from tkinter import ttk
from ui.styles.tk_theme import COLORS, FONTS


class CalibrationPage(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="SENSOR CALIBRATION", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        # Sensor calibration cards in 2x2 grid
        grid = tk.Frame(self, bg=COLORS["bg_dark"])
        grid.pack(fill=tk.BOTH, expand=True, padx=16)

        sensors = [
            ("RADAR", "AN/MPQ-64", 87, "Calibrated", COLORS["success"]),
            ("RF SCANNER", "RF-3000", 65, "Needs tuning", COLORS["warning"]),
            ("ACOUSTIC", "Audi M2K", 92, "Calibrated", COLORS["success"]),
            ("LRF", "LRF-2200", 45, "Out of spec", COLORS["accent_red"]),
        ]

        for i, (name, model, cal_pct, status, color) in enumerate(sensors):
            card = tk.Frame(grid, bg=COLORS["bg_card"], bd=1, relief=tk.SUNKEN)
            card.grid(row=i // 2, column=i % 2, sticky="nsew", padx=4, pady=4)

            tk.Label(card, text=name, fg=COLORS["accent_cyan"], bg=COLORS["bg_card"],
                     font=FONTS["bold"]).pack(anchor="w", padx=8, pady=(8, 0))
            tk.Label(card, text=f"Model: {model}", fg=COLORS["text_muted"],
                     bg=COLORS["bg_card"], font=FONTS["small"]).pack(anchor="w", padx=8)

            # Progress bar frame
            pb_frame = tk.Frame(card, bg=COLORS["bg_input"], height=12)
            pb_frame.pack(fill=tk.X, padx=8, pady=4)
            pb_fill = tk.Frame(pb_frame, bg=color, height=12, width=int(cal_pct * 1.8))
            pb_fill.pack(side=tk.LEFT)

            status_lbl = tk.Label(card, text=f"Status: {status}", fg=color,
                                  bg=COLORS["bg_card"], font=FONTS["default"])
            status_lbl.pack(anchor="w", padx=8)

            tk.Button(card, text="CALIBRATE", bg=COLORS["accent_blue"], fg="white",
                      font=FONTS["bold"], relief=tk.FLAT, padx=12, pady=4, cursor="hand2").pack(pady=8)

        grid.rowconfigure(0, weight=1)
        grid.rowconfigure(1, weight=1)
        grid.columnconfigure(0, weight=1)
        grid.columnconfigure(1, weight=1)

        # System calibration
        sys_frame = tk.LabelFrame(self, text="SYSTEM CALIBRATION", bg=COLORS["bg_card"],
                                   fg=COLORS["accent_cyan"], font=FONTS["bold"],
                                   bd=1, relief=tk.SUNKEN)
        sys_frame.pack(fill=tk.X, padx=16, pady=8)

        for txt in ["Sensor alignment: Complete", "GPS reference: Locked", "Magnetic declination: 12.3°E"]:
            tk.Label(sys_frame, text=txt, fg=COLORS["text_secondary"], bg=COLORS["bg_card"],
                     font=FONTS["default"]).pack(anchor="w", padx=12, pady=2)

        # Progress bar
        pb2_frame = tk.Frame(sys_frame, bg=COLORS["bg_input"], height=12)
        pb2_frame.pack(fill=tk.X, padx=12, pady=4)
        tk.Frame(pb2_frame, bg=COLORS["success"], height=12, width=180).pack(side=tk.LEFT)

        tk.Button(sys_frame, text="RUN FULL SYSTEM CALIBRATION", bg=COLORS["accent_blue"],
                  fg="white", font=FONTS["bold"], relief=tk.FLAT, padx=12, pady=4,
                  cursor="hand2").pack(pady=8)