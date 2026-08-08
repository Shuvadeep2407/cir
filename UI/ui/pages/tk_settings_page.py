"""Settings page for Tkinter"""
import tkinter as tk
from tkinter import ttk
from ui.styles.tk_theme import COLORS, FONTS


class SettingsPage(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="SYSTEM SETTINGS", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        # Tabs
        tab_frame = tk.Frame(self, bg=COLORS["bg_dark"])
        tab_frame.pack(fill=tk.BOTH, expand=True, padx=16)

        notebook = ttk.Notebook(tab_frame)
        notebook.pack(fill=tk.BOTH, expand=True)

        style = ttk.Style()
        style.theme_use("default")
        style.configure("TNotebook", background=COLORS["bg_dark"], borderwidth=0)
        style.configure("TNotebook.Tab", background=COLORS["bg_secondary"],
                        foreground=COLORS["text_secondary"], padding=[12, 6])
        style.map("TNotebook.Tab", background=[("selected", COLORS["bg_primary"])],
                  foreground=[("selected", COLORS["accent_cyan"])])

        # General
        general = tk.Frame(notebook, bg=COLORS["bg_primary"])
        notebook.add(general, text="GENERAL")
        self._add_row(general, "System Name:", "MED-FLIGHT C-UAS-01")
        self._add_row(general, "Operator ID:", "OP-001")
        self._add_row(general, "Location:", tk.StringVar(value="Los Angeles, CA"))
        self._add_row(general, "Auto-start sensors:", tk.Checkbutton, "check")
        self._add_spin(general, "Default map zoom:", 14, 1, 20)
        self._add_spin(general, "Data retention (days):", 90, 1, 365)

        # Sensors
        sensors = tk.Frame(notebook, bg=COLORS["bg_primary"])
        notebook.add(sensors, text="SENSORS")
        self._add_spin(sensors, "Radar range (m):", 5000, 100, 20000, 100)
        self._add_combo(sensors, "RF scan band (MHz):", ["100", "200", "400", "800"])
        self._add_spin(sensors, "Acoustic sensitivity:", 40, 10, 100)
        self._add_spin(sensors, "LRF max range (m):", 3000, 100, 10000)
        self._add_spin(sensors, "Camera framerate (fps):", 30, 5, 60)
        self._add_row(sensors, "Enable sensor fusion:", tk.Checkbutton, "check")

        # AI
        ai = tk.Frame(notebook, bg=COLORS["bg_primary"])
        notebook.add(ai, text="AI ENGINE")
        self._add_row(ai, "Auto-classification:", tk.Checkbutton, "check")
        self._add_row(ai, "Trajectory prediction:", tk.Checkbutton, "check")
        self._add_spin(ai, "Confidence threshold:", 70, 0, 100)
        self._add_spin(ai, "YOLO detection interval:", 100, 33, 1000)

        # Buttons
        btn_frame = tk.Frame(self, bg=COLORS["bg_dark"])
        btn_frame.pack(fill=tk.X, padx=16, pady=12)
        tk.Button(btn_frame, text="SAVE SETTINGS", bg=COLORS["accent_blue"], fg="white",
                  font=FONTS["bold"], relief=tk.FLAT, padx=20, pady=8, cursor="hand2").pack(side=tk.RIGHT, padx=4)
        tk.Button(btn_frame, text="RESET TO DEFAULT", bg=COLORS["accent_red"], fg="white",
                  font=FONTS["bold"], relief=tk.FLAT, padx=20, pady=8, cursor="hand2").pack(side=tk.RIGHT, padx=4)

    def _add_row(self, parent, label, widget_type="entry", check=False):
        f = tk.Frame(parent, bg=COLORS["bg_primary"])
        f.pack(fill=tk.X, padx=16, pady=4)
        tk.Label(f, text=label, fg=COLORS["text_secondary"], bg=COLORS["bg_primary"],
                 font=FONTS["default"], width=25, anchor="w").pack(side=tk.LEFT)
        if widget_type == "entry":
            tk.Entry(f, bg=COLORS["bg_input"], fg=COLORS["text_primary"],
                     font=FONTS["default"], bd=1, relief=tk.SUNKEN, width=30).pack(side=tk.RIGHT)
        elif check:
            cb = tk.Checkbutton(f, bg=COLORS["bg_primary"], fg=COLORS["text_primary"],
                                selectcolor=COLORS["bg_primary"])
            cb.pack(side=tk.RIGHT)

    def _add_spin(self, parent, label, default, min_val, max_val, step=1):
        f = tk.Frame(parent, bg=COLORS["bg_primary"])
        f.pack(fill=tk.X, padx=16, pady=4)
        tk.Label(f, text=label, fg=COLORS["text_secondary"], bg=COLORS["bg_primary"],
                 font=FONTS["default"], width=25, anchor="w").pack(side=tk.LEFT)
        tk.Spinbox(f, from_=min_val, to=max_val, increment=step, width=10,
                   bg=COLORS["bg_input"], fg=COLORS["text_primary"],
                   font=FONTS["default"], bd=1, relief=tk.SUNKEN).pack(side=tk.RIGHT)

    def _add_combo(self, parent, label, values):
        f = tk.Frame(parent, bg=COLORS["bg_primary"])
        f.pack(fill=tk.X, padx=16, pady=4)
        tk.Label(f, text=label, fg=COLORS["text_secondary"], bg=COLORS["bg_primary"],
                 font=FONTS["default"], width=25, anchor="w").pack(side=tk.LEFT)
        ttk.Combobox(f, values=values, state="readonly", width=27).pack(side=tk.RIGHT)