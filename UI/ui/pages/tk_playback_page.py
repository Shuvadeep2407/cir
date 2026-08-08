"""Playback page for Tkinter"""
import tkinter as tk
from ui.styles.tk_theme import COLORS, FONTS


class PlaybackPage(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="MISSION PLAYBACK", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        # Recording list
        list_frame = tk.LabelFrame(self, text="RECORDED MISSIONS", bg=COLORS["bg_card"],
                                    fg=COLORS["accent_cyan"], font=FONTS["bold"],
                                    bd=1, relief=tk.SUNKEN)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=16, pady=4)

        recording_list = tk.Frame(list_frame, bg=COLORS["bg_card"])
        recording_list.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        missions = [
            ("MISSION-20260624-001", "2026-06-24 08:00", "02:15:00", "3 targets"),
            ("MISSION-20260623-001", "2026-06-23 14:30", "01:45:00", "2 targets"),
            ("MISSION-20260622-001", "2026-06-22 10:00", "03:30:00", "5 targets"),
        ]

        for name, date, duration, targets in missions:
            mf = tk.Frame(recording_list, bg=COLORS["bg_primary"], bd=1, relief=tk.SUNKEN)
            mf.pack(fill=tk.X, pady=2)
            tk.Label(mf, text=name, fg=COLORS["accent_cyan"], bg=COLORS["bg_primary"],
                     font=FONTS["bold"]).pack(side=tk.LEFT, padx=8, pady=6)
            tk.Label(mf, text=f"{date} | {duration} | {targets}",
                     fg=COLORS["text_muted"], bg=COLORS["bg_primary"],
                     font=FONTS["small"]).pack(side=tk.LEFT, padx=8, pady=6)
            tk.Button(mf, text="PLAY", bg=COLORS["accent_green"], fg="black",
                      font=FONTS["bold"], relief=tk.FLAT, padx=10, cursor="hand2").pack(side=tk.RIGHT, padx=4, pady=4)

        # Playback controls
        controls = tk.LabelFrame(self, text="PLAYBACK CONTROLS", bg=COLORS["bg_card"],
                                  fg=COLORS["accent_cyan"], font=FONTS["bold"],
                                  bd=1, relief=tk.SUNKEN)
        controls.pack(fill=tk.X, padx=16, pady=8)

        btn_frame = tk.Frame(controls, bg=COLORS["bg_card"])
        btn_frame.pack(pady=8)
        for text, color in [("⏮", COLORS["bg_tertiary"]), ("▶ PLAY", COLORS["accent_green"]),
                            ("⏸", COLORS["accent_yellow"]), ("⏭", COLORS["bg_tertiary"])]:
            tk.Button(btn_frame, text=text, bg=color,
                      fg="white" if color != COLORS["accent_green"] else "black",
                      font=FONTS["bold"], relief=tk.FLAT, padx=12, pady=6, cursor="hand2").pack(side=tk.LEFT, padx=4)

        # Timeline
        timeline = tk.Frame(controls, bg=COLORS["bg_card"])
        timeline.pack(fill=tk.X, padx=16, pady=(0, 12))
        tk.Frame(timeline, bg=COLORS["bg_input"], height=6).pack(fill=tk.X)
        tk.Label(timeline, text="00:00 / 02:15:00", fg=COLORS["text_muted"],
                 bg=COLORS["bg_card"], font=FONTS["small"]).pack()