"""
MED-FLIGHT Counter-UAS Detection System
Military dark theme manager
"""
from PySide6.QtGui import QColor, QPalette, QFont
from PySide6.QtWidgets import QApplication
from utils.constants import COLORS


class ThemeManager:
    """Manages the military dark UI theme"""

    STYLESHEET = f"""
    QMainWindow, QDialog {{
        background-color: {COLORS["bg_dark"]};
        color: {COLORS["text_primary"]};
        font-family: 'Segoe UI', 'Consolas', monospace;
        font-size: 12px;
    }}
    QWidget {{
        background-color: transparent;
        color: {COLORS["text_primary"]};
        font-size: 12px;
    }}
    QLabel#pageTitle {{
        font-size: 18px; font-weight: bold; color: {COLORS["accent_cyan"]};
        padding: 8px 0; border-bottom: 1px solid {COLORS["border"]}; margin-bottom: 12px;
    }}
    QLabel#sectionTitle {{
        font-size: 14px; font-weight: bold; color: {COLORS["text_primary"]}; padding: 4px 0;
    }}
    QLabel#statusLabel {{
        font-size: 11px; color: {COLORS["text_secondary"]}; padding: 2px;
    }}
    QLabel#valueLabel {{
        font-size: 13px; font-weight: bold; color: {COLORS["accent_green"]}; font-family: 'Consolas', monospace;
    }}
    QPushButton {{
        background-color: {COLORS["bg_tertiary"]}; color: {COLORS["text_primary"]};
        border: 1px solid {COLORS["border"]}; border-radius: 4px;
        padding: 6px 16px; font-size: 12px; font-weight: bold; min-height: 28px;
    }}
    QPushButton:hover {{
        background-color: {COLORS["bg_secondary"]}; border-color: {COLORS["border_active"]};
    }}
    QPushButton:pressed {{
        background-color: {COLORS["bg_primary"]}; border-color: {COLORS["accent_cyan"]};
    }}
    QPushButton:disabled {{
        background-color: {COLORS["bg_primary"]}; color: {COLORS["text_muted"]}; border-color: {COLORS["border"]};
    }}
    QPushButton#primaryButton {{
        background-color: {COLORS["accent_blue"]}; border: 1px solid {COLORS["accent_blue"]}; color: white;
    }}
    QPushButton#primaryButton:hover {{ background-color: #0055dd; }}
    QPushButton#dangerButton {{
        background-color: {COLORS["accent_red"]}; border: 1px solid {COLORS["accent_red"]}; color: white;
    }}
    QPushButton#dangerButton:hover {{ background-color: #dd0000; }}
    QPushButton#successButton {{
        background-color: {COLORS["accent_green"]}; border: 1px solid {COLORS["accent_green"]}; color: #000;
    }}
    QPushButton#sidebarButton {{
        background-color: transparent; border: none; border-radius: 0;
        text-align: left; padding: 10px 16px; font-size: 12px;
        color: {COLORS["text_secondary"]}; border-left: 3px solid transparent;
    }}
    QPushButton#sidebarButton:hover {{
        background-color: {COLORS["bg_secondary"]}; color: {COLORS["text_primary"]};
        border-left: 3px solid {COLORS["accent_cyan"]};
    }}
    QPushButton#sidebarButton:checked {{
        background-color: {COLORS["bg_tertiary"]}; color: {COLORS["accent_cyan"]};
        border-left: 3px solid {COLORS["accent_cyan"]};
    }}
    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {{
        background-color: {COLORS["bg_input"]}; color: {COLORS["text_primary"]};
        border: 1px solid {COLORS["border"]}; border-radius: 4px;
        padding: 6px 10px; font-size: 12px; min-height: 24px;
    }}
    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {{
        border-color: {COLORS["border_active"]};
    }}
    QComboBox::drop-down {{ border: none; background-color: {COLORS["bg_tertiary"]}; width: 20px; }}
    QComboBox QAbstractItemView {{
        background-color: {COLORS["bg_primary"]}; color: {COLORS["text_primary"]};
        border: 1px solid {COLORS["border"]}; selection-background-color: {COLORS["bg_tertiary"]};
    }}
    QTextEdit, QPlainTextEdit {{
        background-color: {COLORS["bg_input"]}; color: {COLORS["text_primary"]};
        border: 1px solid {COLORS["border"]}; border-radius: 4px;
        padding: 6px; font-family: 'Consolas', monospace; font-size: 11px;
    }}
    QTableWidget, QTreeWidget {{
        background-color: {COLORS["bg_primary"]}; color: {COLORS["text_primary"]};
        border: 1px solid {COLORS["border"]}; border-radius: 4px;
        gridline-color: {COLORS["border"]}; font-size: 11px;
    }}
    QTableWidget::item, QTreeWidget::item {{
        padding: 6px 8px; border-bottom: 1px solid {COLORS["border"]};
    }}
    QTableWidget::item:selected, QTreeWidget::item:selected {{
        background-color: {COLORS["bg_tertiary"]}; color: {COLORS["accent_cyan"]};
    }}
    QHeaderView::section {{
        background-color: {COLORS["bg_secondary"]}; color: {COLORS["text_secondary"]};
        padding: 8px; border: none; border-bottom: 2px solid {COLORS["border"]};
        font-weight: bold; font-size: 11px;
    }}
    QListWidget {{
        background-color: {COLORS["bg_primary"]}; color: {COLORS["text_primary"]};
        border: 1px solid {COLORS["border"]}; border-radius: 4px;
    }}
    QListWidget::item {{ padding: 8px 12px; border-bottom: 1px solid {COLORS["border"]}; }}
    QListWidget::item:selected {{
        background-color: {COLORS["bg_tertiary"]}; color: {COLORS["accent_cyan"]};
    }}
    QScrollBar:vertical {{
        background: {COLORS["bg_primary"]}; width: 8px; margin: 0;
    }}
    QScrollBar::handle:vertical {{
        background: {COLORS["bg_tertiary"]}; border-radius: 4px; min-height: 30px;
    }}
    QScrollBar::handle:vertical:hover {{ background: {COLORS["border_active"]}; }}
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{ height: 0; }}
    QScrollBar:horizontal {{
        background: {COLORS["bg_primary"]}; height: 8px;
    }}
    QScrollBar::handle:horizontal {{
        background: {COLORS["bg_tertiary"]}; border-radius: 4px; min-width: 30px;
    }}
    QGroupBox {{
        background-color: {COLORS["bg_card"]}; border: 1px solid {COLORS["border"]};
        border-radius: 6px; margin-top: 12px; padding: 16px 12px 12px 12px;
    }}
    QGroupBox::title {{
        subcontrol-origin: margin; subcontrol-position: top left;
        padding: 2px 10px; background-color: {COLORS["bg_card"]}; color: {COLORS["accent_cyan"]};
    }}
    QProgressBar {{
        background-color: {COLORS["bg_input"]}; border: 1px solid {COLORS["border"]};
        border-radius: 3px; text-align: center; color: {COLORS["text_primary"]};
        font-size: 10px; min-height: 16px;
    }}
    QProgressBar::chunk {{ background-color: {COLORS["accent_blue"]}; border-radius: 2px; }}
    QTabWidget::pane {{ background-color: {COLORS["bg_primary"]}; border: 1px solid {COLORS["border"]}; border-radius: 4px; }}
    QTabBar::tab {{
        background-color: {COLORS["bg_secondary"]}; color: {COLORS["text_secondary"]};
        padding: 8px 18px; border: 1px solid {COLORS["border"]}; border-bottom: none;
        border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 2px;
    }}
    QTabBar::tab:selected {{
        background-color: {COLORS["bg_primary"]}; color: {COLORS["accent_cyan"]};
        border-bottom: 2px solid {COLORS["accent_cyan"]};
    }}
    QSlider::groove:horizontal {{ background: {COLORS["bg_input"]}; height: 4px; border-radius: 2px; }}
    QSlider::handle:horizontal {{
        background: {COLORS["accent_cyan"]}; width: 14px; height: 14px;
        margin: -5px 0; border-radius: 7px;
    }}
    QSlider::sub-page:horizontal {{ background: {COLORS["accent_blue"]}; border-radius: 2px; }}
    QSplitter::handle {{ background-color: {COLORS["border"]}; width: 1px; height: 1px; }}
    QSplitter::handle:hover {{ background-color: {COLORS["border_active"]}; }}
    """

    @classmethod
    def apply(cls, app: QApplication):
        app.setStyle("Fusion")
        palette = QPalette()
        palette.setColor(QPalette.Window, QColor(COLORS["bg_dark"]))
        palette.setColor(QPalette.WindowText, QColor(COLORS["text_primary"]))
        palette.setColor(QPalette.Base, QColor(COLORS["bg_primary"]))
        palette.setColor(QPalette.AlternateBase, QColor(COLORS["bg_secondary"]))
        palette.setColor(QPalette.ToolTipBase, QColor(COLORS["bg_tertiary"]))
        palette.setColor(QPalette.ToolTipText, QColor(COLORS["text_primary"]))
        palette.setColor(QPalette.Text, QColor(COLORS["text_primary"]))
        palette.setColor(QPalette.Button, QColor(COLORS["bg_tertiary"]))
        palette.setColor(QPalette.ButtonText, QColor(COLORS["text_primary"]))
        palette.setColor(QPalette.BrightText, QColor(COLORS["accent_red"]))
        palette.setColor(QPalette.Link, QColor(COLORS["accent_cyan"]))
        palette.setColor(QPalette.Highlight, QColor(COLORS["accent_blue"]))
        palette.setColor(QPalette.HighlightedText, QColor("#ffffff"))
        app.setPalette(palette)
        app.setStyleSheet(cls.STYLESHEET)
        font = QFont("Segoe UI", 10)
        font.setStyleStrategy(QFont.PreferAntialias)
        app.setFont(font)