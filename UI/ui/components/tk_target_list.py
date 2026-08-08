"""Target list widget for Tkinter"""
import tkinter as tk
from tkinter import ttk
from ui.styles.tk_theme import COLORS, FONTS
from utils.constants import ThreatLevel


class TargetListWidget(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.targets = []
        self.setup_ui()

    def setup_ui(self):
        title = tk.Label(self, text="TARGET TRACKING", fg=COLORS["accent_cyan"],
                         bg=COLORS["bg_dark"], font=FONTS["title"], anchor="w")
        title.pack(fill=tk.X, padx=16, pady=(16, 8))

        # Treeview
        container = tk.Frame(self, bg=COLORS["bg_dark"])
        container.pack(fill=tk.BOTH, expand=True, padx=16, pady=8)

        columns = ("ID", "Type", "Speed", "Alt", "Threat", "Status", "Confidence")
        self.tree = ttk.Treeview(container, columns=columns, show="headings",
                                 selectmode="browse")
        for col in columns:
            self.tree.heading(col, text=col)
            self.tree.column(col, width=100, anchor="center")

        self.tree.column("ID", width=120)
        self.tree.column("Type", width=140)

        vsb = ttk.Scrollbar(container, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=vsb.set)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)

        # Style the tree
        style = ttk.Style()
        style.theme_use("default")
        style.configure("Treeview",
                        background=COLORS["bg_primary"],
                        foreground=COLORS["text_primary"],
                        fieldbackground=COLORS["bg_primary"],
                        font=FONTS["default"],
                        borderwidth=0)
        style.configure("Treeview.Heading",
                        background=COLORS["bg_secondary"],
                        foreground=COLORS["text_secondary"],
                        font=FONTS["bold"],
                        borderwidth=1)

    def update_targets(self, targets):
        self.targets = targets
        for row in self.tree.get_children():
            self.tree.delete(row)
        for t in targets:
            threat_color = t.threat_level.color if hasattr(t.threat_level, 'color') else COLORS["text_muted"]
            self.tree.insert("", tk.END,
                             values=(t.target_id, t.classification or "Unknown",
                                     f"{t.speed_ms:.1f}", f"{t.alt_m:.0f}",
                                     t.threat_level.name if hasattr(t.threat_level, 'name') else "NONE",
                                     "TRACKING", f"{t.classification_confidence:.0%}"))