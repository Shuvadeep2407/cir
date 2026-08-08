"""Camera video panel for Tkinter"""
import tkinter as tk
from ui.styles.tk_theme import COLORS, FONTS
from core.simulators import CameraSimulator
import numpy as np


class CameraView(tk.Canvas):
    """Single camera view with simulated feed"""
    def __init__(self, parent, name, mode="EO", **kw):
        super().__init__(parent, bg="#0a0e14", highlightthickness=0,
                         bd=1, relief=tk.SUNKEN, **kw)
        self.name = name
        self.mode = mode
        self.simulator = CameraSimulator(mode)
        self.frame = None
        self.min_height = 120

    def update_frame(self):
        self.frame = self.simulator.get_frame()
        self.render()

    def render(self):
        self.delete("all")
        w = self.winfo_width()
        h = self.winfo_height()
        if w < 20 or h < 20:
            return

        # Background
        self.create_rectangle(0, 0, w, h, fill="#0a0e14", outline=COLORS["border"])

        # Header
        icon = "📷" if self.mode != "IR" else "🌡"
        self.create_text(6, 10, text=f"{icon} {self.name}", fill=COLORS["accent_cyan"],
                         font=FONTS["small"], anchor="w")
        self.create_text(w - 6, 10, text="●", fill=COLORS["success"],
                         font=FONTS["small"], anchor="e")

        # Simulated frame area
        y_off = 22
        fw = w - 4
        fh = h - y_off - 4
        if fw > 20 and fh > 20:
            self.create_rectangle(2, y_off, 2 + fw, y_off + fh,
                                  fill="#0d1520", outline="")

            # Draw crosshairs
            cx, cy = 2 + fw // 2, y_off + fh // 2
            self.create_line(cx - 20, cy, cx + 20, cy, fill=COLORS["accent_red"], width=1)
            self.create_line(cx, cy - 20, cx, cy + 20, fill=COLORS["accent_red"], width=1)

            # Mode label
            self.create_text(6, y_off + 12, text=f"{self.mode} | {fw}x{fh}",
                             fill=COLORS["accent_green"], font=FONTS["small"], anchor="w")

            # Random noise dots (simulated detections)
            import random
            for _ in range(random.randint(0, 5)):
                dx = random.randint(10, fw - 10)
                dy = random.randint(y_off + 10, y_off + fh - 10)
                self.create_oval(2 + dx - 2, dy - 2, 2 + dx + 2, dy + 2,
                                 fill=COLORS["accent_red"], outline="")


class VideoPanel(tk.Frame):
    """2x2 camera grid"""
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)

        # Header
        header = tk.Frame(self, bg=COLORS["bg_dark"])
        header.pack(fill=tk.X)
        tk.Label(header, text="VIDEO FEEDS", fg=COLORS["text_primary"],
                 bg=COLORS["bg_dark"], font=FONTS["section"]).pack(side=tk.LEFT)

        self.rec_btn = tk.Button(header, text="● REC", bg=COLORS["accent_red"], fg="white",
                                 font=FONTS["bold"], relief=tk.FLAT, padx=10, cursor="hand2")
        self.rec_btn.pack(side=tk.RIGHT, padx=4)

        # Camera grid
        grid = tk.Frame(self, bg=COLORS["bg_dark"])
        grid.pack(fill=tk.BOTH, expand=True)

        cams = [
            ("EO CAM-1", "EO"), ("IR CAM-1", "IR"),
            ("EO CAM-2", "EO"), ("IR CAM-2", "IR"),
        ]

        self.cameras = []
        positions = [(0, 0), (0, 1), (1, 0), (1, 1)]
        for (name, mode), (row, col) in zip(cams, positions):
            cam = CameraView(grid, name, mode)
            cam.grid(row=row, column=col, sticky="nsew", padx=1, pady=1)
            self.cameras.append(cam)

        grid.rowconfigure(0, weight=1)
        grid.rowconfigure(1, weight=1)
        grid.columnconfigure(0, weight=1)
        grid.columnconfigure(1, weight=1)

        self._update_timer = None
        self.start()

    def start(self):
        def update():
            for cam in self.cameras:
                cam.update_frame()
            self._update_timer = self.after(100, update)
        update()

    def stop(self):
        if self._update_timer:
            self.after_cancel(self._update_timer)