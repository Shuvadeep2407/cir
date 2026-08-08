"""Alerts page for Tkinter"""
import tkinter as tk
from tkinter import ttk
from ui.styles.tk_theme import COLORS, FONTS


class AlertsPage(tk.Frame):
    def __init__(self, parent, alert_manager=None, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.alert_manager = alert_manager
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="ALERTS & NOTIFICATIONS", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        # Controls
        controls = tk.Frame(self, bg=COLORS["bg_dark"])
        controls.pack(fill=tk.X, padx=16, pady=4)

        self.filter_var = tk.StringVar(value="ALL")
        self.filter_combo = ttk.Combobox(controls, textvariable=self.filter_var,
                                          values=["ALL", "EMERGENCY", "CRITICAL", "WARNING", "INFO"],
                                          state="readonly", width=15)
        self.filter_combo.pack(side=tk.LEFT, padx=4)
        self.filter_combo.bind("<<ComboboxSelected>>", lambda e: self._refresh())

        self.ack_btn = tk.Button(controls, text="ACKNOWLEDGE",
                                 bg=COLORS["accent_blue"], fg="white",
                                 font=FONTS["bold"], relief=tk.FLAT, padx=12, cursor="hand2",
                                 command=self._acknowledge_selected)
        self.ack_btn.pack(side=tk.RIGHT, padx=4)

        self.clear_btn = tk.Button(controls, text="CLEAR ALL",
                                   bg=COLORS["accent_red"], fg="white",
                                   font=FONTS["bold"], relief=tk.FLAT, padx=12, cursor="hand2",
                                   command=self._clear)
        self.clear_btn.pack(side=tk.RIGHT, padx=4)

        # Table
        container = tk.Frame(self, bg=COLORS["bg_dark"])
        container.pack(fill=tk.BOTH, expand=True, padx=16, pady=8)

        columns = ("TIME", "SEVERITY", "SOURCE", "TITLE", "MESSAGE", "STATUS")
        self.tree = ttk.Treeview(container, columns=columns, show="headings", selectmode="browse")
        for col in columns:
            self.tree.heading(col, text=col)
            self.tree.column(col, width=100, anchor="w")
        self.tree.column("TIME", width=80)
        self.tree.column("SEVERITY", width=90)
        self.tree.column("SOURCE", width=100)
        self.tree.column("TITLE", width=150)

        vsb = ttk.Scrollbar(container, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=vsb.set)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)

        style = ttk.Style()
        style.configure("Treeview", background=COLORS["bg_primary"],
                        foreground=COLORS["text_primary"],
                        fieldbackground=COLORS["bg_primary"],
                        font=FONTS["default"])
        style.configure("Treeview.Heading", background=COLORS["bg_secondary"],
                        foreground=COLORS["text_secondary"], font=FONTS["bold"])

        self._refresh()

    def _refresh(self):
        if not self.alert_manager:
            return
        for row in self.tree.get_children():
            self.tree.delete(row)

        severity_filter = self.filter_var.get()
        alerts = self.alert_manager.alerts
        if severity_filter != "ALL":
            alerts = [a for a in alerts if hasattr(a.severity, 'label') and a.severity.label == severity_filter]

        for a in alerts:
            status = "ACK'D" if a.acknowledged else "NEW"
            ts = a.timestamp.strftime("%H:%M:%S") if hasattr(a.timestamp, 'strftime') else str(a.timestamp)
            sev = a.severity.label if hasattr(a.severity, 'label') else "INFO"
            self.tree.insert("", tk.END, values=(ts, sev, a.source, a.title, a.message[:50], status))

    def _acknowledge_selected(self):
        if not self.alert_manager:
            return
        sel = self.tree.selection()
        if sel:
            idx = self.tree.index(sel[0])
            if idx < len(self.alert_manager.alerts):
                alert = self.alert_manager.alerts[idx]
                self.alert_manager.acknowledge_alert(alert.alert_id)
                self._refresh()

    def _clear(self):
        if self.alert_manager:
            self.alert_manager.clear_all()
            self._refresh()