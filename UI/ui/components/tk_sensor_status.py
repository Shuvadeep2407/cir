"""Sensor status panel for Tkinter"""
import tkinter as tk
from ui.styles.tk_theme import COLORS, FONTS


class SensorGauge(tk.Frame):
    """Single sensor status gauge"""
    def __init__(self, parent, name, sensor_type, color, **kw):
        super().__init__(parent, bg=COLORS["bg_card"], bd=1, relief=tk.SUNKEN, **kw)
        self.name = name
        self.sensor_type = sensor_type
        self.gauge_color = color
        self.status = "STANDBY"
        self.value = 0.0
        self.confidence = 0.0
        self.online = False
        self.cpu = 0.0
        self.temp = 35.0
        self.min_height = 120

        self._setup_labels()

    def _setup_labels(self):
        self.name_lbl = tk.Label(self, text=self.name, fg=self.gauge_color,
                                 bg=COLORS["bg_card"], font=FONTS["bold"])
        self.name_lbl.pack(anchor="w", padx=6, pady=(4, 0))

        self.type_lbl = tk.Label(self, text=self.sensor_type, fg=COLORS["text_muted"],
                                 bg=COLORS["bg_card"], font=FONTS["small"])
        self.type_lbl.pack(anchor="w", padx=6)

        self.status_lbl = tk.Label(self, text="○ STANDBY", fg=COLORS["text_muted"],
                                   bg=COLORS["bg_card"], font=FONTS["small"])
        self.status_lbl.pack(anchor="w", padx=6)

        self.value_lbl = tk.Label(self, text="---", fg=self.gauge_color,
                                  bg=COLORS["bg_card"], font=FONTS["huge"])
        self.value_lbl.pack(anchor="w", padx=6)

        # Confidence bar frame
        self.bar_frame = tk.Frame(self, bg=COLORS["bg_input"], height=4)
        self.bar_frame.pack(fill=tk.X, padx=6, pady=2)
        self.bar_fill = tk.Frame(self.bar_frame, bg=self.gauge_color, height=4)
        self.bar_fill.place(x=0, y=0, relwidth=0, height=4)

        self.info_lbl = tk.Label(self, text="CPU:0% | 35°C", fg=COLORS["text_muted"],
                                 bg=COLORS["bg_card"], font=FONTS["small"])
        self.info_lbl.pack(anchor="w", padx=6)

        self.online_lbl = tk.Label(self, text="○ OFFLINE", fg=COLORS["text_muted"],
                                   bg=COLORS["bg_card"], font=FONTS["small"])
        self.online_lbl.pack(anchor="w", padx=6, pady=(0, 4))

    def update_data(self, sensor):
        self.status = sensor.status if hasattr(sensor, 'status') else "STANDBY"
        self.value = getattr(sensor, 'range_m', 0)
        self.confidence = sensor.last_reading.confidence if hasattr(sensor, 'last_reading') and sensor.last_reading else 0
        self.online = getattr(sensor, 'online', False)
        self.cpu = getattr(sensor, 'cpu_usage', 0)
        self.temp = getattr(sensor, 'temperature_c', 35)

        status_color = COLORS["success"] if self.online else COLORS["warning"]
        self.status_lbl.configure(text=f"● {self.status}" if self.online else f"○ {self.status}",
                                  fg=status_color)

        val_text = f"{self.value:.0f}" if self.value > 0 else "---"
        self.value_lbl.configure(text=val_text)

        # Confidence bar
        conf = min(1.0, max(0.0, self.confidence))
        self.bar_fill.place(x=0, y=0, relwidth=conf, height=4)

        self.info_lbl.configure(text=f"CPU:{self.cpu:.0f}% | {self.temp:.0f}°C")
        self.online_lbl.configure(text="● ONLINE" if self.online else "○ OFFLINE",
                                  fg=COLORS["success"] if self.online else COLORS["text_muted"])


class SensorStatusPanel(tk.Frame):
    """Panel showing all sensor gauges"""
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)

        # Header
        header = tk.Frame(self, bg=COLORS["bg_dark"])
        header.pack(fill=tk.X)
        tk.Label(header, text="SENSOR STATUS", fg=COLORS["text_primary"],
                 bg=COLORS["bg_dark"], font=FONTS["section"]).pack(side=tk.LEFT)

        self.gauges = {}

        # Gauge grid
        self.grid_frame = tk.Frame(self, bg=COLORS["bg_dark"])
        self.grid_frame.pack(fill=tk.BOTH, expand=True)

        sensors = [
            ("RADAR_01", "RADAR", "AN/MPQ-64 Radar", COLORS["radar_color"]),
            ("RF_01", "RF", "RF-3000 Scanner", COLORS["rf_color"]),
            ("ACOUSTIC_01", "ACOUSTIC", "Audi Array M2K", COLORS["acoustic_color"]),
            ("LRF_01", "LRF", "LRF-2200 Rangefinder", COLORS["lrf_color"]),
        ]

        for i, (sid, stype, name, color) in enumerate(sensors):
            gauge = SensorGauge(self.grid_frame, name, stype, color)
            gauge.grid(row=i // 2, column=i % 2, sticky="nsew", padx=2, pady=2)
            self.gauges[sid] = gauge

        self.grid_frame.rowconfigure(0, weight=1)
        self.grid_frame.rowconfigure(1, weight=1)
        self.grid_frame.columnconfigure(0, weight=1)
        self.grid_frame.columnconfigure(1, weight=1)

    def update_sensor(self, sensor_id, sensor):
        if sensor_id in self.gauges:
            self.gauges[sensor_id].update_data(sensor)