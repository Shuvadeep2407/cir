"""Users page for Tkinter"""
import tkinter as tk
from tkinter import ttk
from ui.styles.tk_theme import COLORS, FONTS


class UsersPage(tk.Frame):
    def __init__(self, parent, **kw):
        super().__init__(parent, bg=COLORS["bg_dark"], **kw)
        self.setup_ui()

    def setup_ui(self):
        tk.Label(self, text="USER MANAGEMENT", fg=COLORS["accent_cyan"],
                 bg=COLORS["bg_dark"], font=FONTS["title"]).pack(anchor="w", padx=16, pady=(16, 8))

        # Controls
        controls = tk.Frame(self, bg=COLORS["bg_dark"])
        controls.pack(fill=tk.X, padx=16, pady=4)
        tk.Button(controls, text="ADD USER", bg=COLORS["accent_green"], fg="black",
                  font=FONTS["bold"], relief=tk.FLAT, padx=12, cursor="hand2").pack(side=tk.RIGHT)
        tk.Button(controls, text="REMOVE", bg=COLORS["accent_red"], fg="white",
                  font=FONTS["bold"], relief=tk.FLAT, padx=12, cursor="hand2").pack(side=tk.RIGHT, padx=4)

        # Tree
        container = tk.Frame(self, bg=COLORS["bg_dark"])
        container.pack(fill=tk.BOTH, expand=True, padx=16, pady=8)

        columns = ("USERNAME", "ROLE", "STATUS", "LAST LOGIN", "PERMISSIONS")
        self.tree = ttk.Treeview(container, columns=columns, show="headings")
        for col in columns:
            self.tree.heading(col, text=col)
            self.tree.column(col, width=120, anchor="w")

        vsb = ttk.Scrollbar(container, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=vsb.set)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)

        style = ttk.Style()
        style.configure("Treeview", background=COLORS["bg_primary"],
                        foreground=COLORS["text_primary"],
                        fieldbackground=COLORS["bg_primary"])
        style.configure("Treeview.Heading", background=COLORS["bg_secondary"],
                        foreground=COLORS["text_secondary"], font=FONTS["bold"])

        # Sample users
        users = [
            ("admin", "Administrator", "ACTIVE", "2026-06-24 08:15", "Full"),
            ("operator1", "Operator", "ACTIVE", "2026-06-24 07:30", "Read/Write"),
            ("analyst", "Analyst", "ACTIVE", "2026-06-23 14:22", "Read Only"),
            ("guest", "Guest", "DISABLED", "2026-06-20 09:00", "View Only"),
        ]
        for username, role, status, last_login, perms in users:
            self.tree.insert("", tk.END, values=(username, role, status, last_login, perms))