"""Tactical radar map widget for Tkinter"""
import tkinter as tk
import math
import random
from ui.styles.tk_theme import COLORS, FONTS


class TacticalMapWidget(tk.Canvas):
    """Radar display with sweep and targets"""
    def __init__(self, parent, **kw):
        super().__init__(parent, bg="#0a0e14", highlightthickness=0,
                         bd=1, relief=tk.SUNKEN, **kw)
        self.targets = []
        self.rotation = 0.0
        self.center_lat = 34.0522
        self.center_lon = -118.2437

    def set_targets(self, targets):
        self.targets = targets

    def animate(self):
        self.rotation = (self.rotation + 2) % 360
        self.render()
        self.after(50, self.animate)

    def render(self):
        self.delete("all")
        w = self.winfo_width()
        h = self.winfo_height()
        if w < 50 or h < 50:
            return
        cx, cy = w // 2, h // 2
        radius = min(w, h) * 0.42

        # Grid rings
        for r in [0.25, 0.5, 0.75, 1.0]:
            rr = radius * r
            self.create_oval(cx - rr, cy - rr, cx + rr, cy + rr,
                             outline="#1a3050", dash=(4, 4))

        # Crosshairs
        self.create_line(cx - radius, cy, cx + radius, cy, fill="#1a3050")
        self.create_line(cx, cy - radius, cx, cy + radius, fill="#1a3050")

        # Range labels
        for i, dist in enumerate(["1km", "2km", "3km", "4km"], 1):
            r = radius * (i * 0.25)
            self.create_text(cx + r, cy - 5, text=dist, fill=COLORS["text_muted"],
                             font=FONTS["small"], anchor="w")

        # Radar sweep
        angle_rad = math.radians(self.rotation)
        sweep_end_x = cx + radius * 1.05 * math.cos(angle_rad)
        sweep_end_y = cy - radius * 1.05 * math.sin(angle_rad)

        # Sweep triangle (semi-transparent)
        sweep_angle = math.radians(45)
        sx1 = cx + radius * 0.05 * math.cos(angle_rad - sweep_angle)
        sy1 = cy - radius * 0.05 * math.sin(angle_rad - sweep_angle)
        sx2 = cx + radius * 1.0 * math.cos(angle_rad)
        sy2 = cy - radius * 1.0 * math.sin(angle_rad)
        sx3 = cx + radius * 0.05 * math.cos(angle_rad + sweep_angle)
        sy3 = cy - radius * 0.05 * math.sin(angle_rad + sweep_angle)

        self.create_polygon(cx, cy, sx2, sy2, sx3, sy3,
                            fill=COLORS["accent_green"], stipple="gray25",
                            outline="")

        # Sweep line
        self.create_line(cx, cy, sweep_end_x, sweep_end_y,
                         fill=COLORS["accent_green"], width=2)

        # Random noise blips
        for _ in range(8):
            a = random.uniform(0, 360)
            d = random.uniform(0.1, 1.0) * radius
            bx = cx + d * math.cos(math.radians(a))
            by = cy - d * math.sin(math.radians(a))
            self.create_oval(bx - 2, by - 2, bx + 2, by + 2,
                             fill=COLORS["accent_green"], outline="")

        # Targets
        for target in self.targets:
            dx = (target.lon - self.center_lon) * 50000
            dy = (target.lat - self.center_lat) * 50000
            tx = cx + dx
            ty = cy - dy

            if tx < 0 or tx > w or ty < 0 or ty > h:
                continue

            color = target.threat_level.color if hasattr(target.threat_level, 'color') else COLORS["accent_red"]

            # Diamond marker
            size = 12 if hasattr(target.threat_level, 'name') and target.threat_level.name == "HIGH" else 8
            self.create_polygon(tx, ty - size, tx + size, ty,
                                tx, ty + size, tx - size, ty,
                                fill="", outline=color, width=2)

            # Label
            self.create_text(tx + size + 4, ty + 4, text=target.target_id,
                             fill=color, font=FONTS["small"], anchor="w")

        # Center marker
        self.create_line(cx - 10, cy, cx + 10, cy, fill=COLORS["accent_cyan"], width=2)
        self.create_line(cx, cy - 10, cx, cy + 10, fill=COLORS["accent_cyan"], width=2)

        # Compass
        self.create_text(cx, 20, text="N", fill=COLORS["text_secondary"], font=FONTS["small"])
        self.create_text(w - 20, cy, text="E", fill=COLORS["text_secondary"], font=FONTS["small"])
        self.create_text(cx, h - 8, text="S", fill=COLORS["text_secondary"], font=FONTS["small"])
        self.create_text(10, cy, text="W", fill=COLORS["text_secondary"], font=FONTS["small"])

        # Legend
        legend_y = 30
        for name, color in [("HIGH", COLORS["accent_red"]), ("MEDIUM", COLORS["accent_orange"]),
                            ("LOW", COLORS["accent_yellow"]), ("NONE", COLORS["success"])]:
            self.create_rectangle(10, legend_y, 18, legend_y + 8, fill=color, outline="")
            self.create_text(22, legend_y + 4, text=name, fill=COLORS["text_muted"],
                             font=FONTS["small"], anchor="w")
            legend_y += 16