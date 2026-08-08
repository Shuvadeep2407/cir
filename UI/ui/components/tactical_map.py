"""
MED-FLIGHT Counter-UAS Detection System
Tactical map widget with target plotting
"""
import numpy as np
from PySide6.QtCore import Qt, QTimer, Signal, QRectF, QPointF
from PySide6.QtGui import QPainter, QPen, QColor, QBrush, QFont, QPainterPath
from PySide6.QtWidgets import QWidget, QVBoxLayout, QLabel
from utils.constants import COLORS, DEFAULT_LAT, DEFAULT_LON, DEFAULT_ZOOM, ThreatLevel


class TacticalMapWidget(QWidget):
    """Tactical situation display with radar sweep and targets"""
    target_selected = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(400, 300)
        self.targets = []
        self.selected_target = None
        self.rotation = 0.0
        self.zoom = 1.0
        self.pan_x = 0.0
        self.pan_y = 0.0
        self.center_lat = DEFAULT_LAT
        self.center_lon = DEFAULT_LON

        self._animate_timer = QTimer(self)
        self._animate_timer.timeout.connect(self._animate)
        self._animate_timer.start(50)

    def set_targets(self, targets: list):
        self.targets = targets
        self.update()

    def _animate(self):
        self.rotation = (self.rotation + 0.5) % 360
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        w, h = self.width(), self.height()
        cx, cy = w // 2, h // 2
        radius = min(w, h) * 0.42

        # Background
        painter.fillRect(0, 0, w, h, QColor("#0a0e14"))

        # Grid rings
        for r in [0.25, 0.5, 0.75, 1.0]:
            ring_r = radius * r
            painter.setPen(QPen(QColor("#1a3050"), 1, Qt.DashLine))
            painter.drawEllipse(QPointF(cx, cy), ring_r, ring_r)

        # Crosshairs
        painter.setPen(QPen(QColor("#1a3050"), 1))
        painter.drawLine(cx - radius, cy, cx + radius, cy)
        painter.drawLine(cx, cy - radius, cx, cy + radius)

        # Range labels
        painter.setPen(QColor(COLORS["text_muted"]))
        painter.setFont(QFont("Consolas", 8))
        for i, dist in enumerate(["1km", "2km", "3km", "4km"], 1):
            r = radius * (i * 0.25)
            painter.drawText(int(cx + r), int(cy - 5), dist)

        # Radar sweep
        painter.save()
        painter.translate(cx, cy)
        painter.rotate(self.rotation)

        # Sweep arc
        sweep_path = QPainterPath()
        sweep_path.moveTo(0, 0)
        sweep_path.arcTo(-radius, -radius, radius * 2, radius * 2, -45, 90)
        sweep_path.closeSubpath()
        gradient = QColor(COLORS["accent_green"])
        gradient.setAlpha(30)
        painter.fillPath(sweep_path, QBrush(gradient))

        # Sweep line
        painter.setPen(QPen(QColor(COLORS["accent_green"]), 2))
        painter.drawLine(0, 0, int(radius * 1.05), 0)

        # Rotating blips
        painter.setPen(QPen(QColor(COLORS["accent_green"]), 1))
        painter.setBrush(QBrush(QColor(COLORS["accent_green"])))
        for _ in range(6):
            angle = np.random.uniform(0, 360)
            dist = np.random.uniform(0.1, 1.0) * radius
            painter.drawEllipse(QPointF(
                dist * np.cos(np.radians(angle)),
                dist * np.sin(np.radians(angle))
            ), 2, 2)

        painter.restore()

        # Plot targets
        for target in self.targets:
            # Convert lat/lon to screen coords (simplified)
            dx = (target.lon - self.center_lon) * 50000 * self.zoom
            dy = (target.lat - self.center_lat) * 50000 * self.zoom
            tx = int(cx + dx + self.pan_x)
            ty = int(cy - dy + self.pan_y)

            if tx < 0 or tx > w or ty < 0 or ty > h:
                continue

            # Target color by threat
            color = QColor(target.threat_level.color)
            
            # Target icon
            painter.setPen(QPen(color, 2))
            painter.setBrush(QBrush(QColor(color.red(), color.green(), color.blue(), 80)))
            
            # Diamond shape
            size = 8 if target.threat_level != ThreatLevel.HIGH else 12
            path = QPainterPath()
            path.moveTo(tx, ty - size)
            path.lineTo(tx + size, ty)
            path.lineTo(tx, ty + size)
            path.lineTo(tx - size, ty)
            path.closeSubpath()
            painter.drawPath(path)

            # Label
            painter.setPen(color)
            painter.setFont(QFont("Consolas", 8))
            painter.drawText(tx + size + 4, ty + 4, target.target_id)

            # Trajectory prediction
            if len(target.track_points) > 2:
                painter.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 100), 1, Qt.DashLine))
                points = target.get_track_array()[-10:]
                for i in range(len(points) - 1):
                    sp_x = cx + (points[i][1] - self.center_lon) * 50000 * self.zoom + self.pan_x
                    sp_y = cy - (points[i][0] - self.center_lat) * 50000 * self.zoom + self.pan_y
                    ep_x = cx + (points[i+1][1] - self.center_lon) * 50000 * self.zoom + self.pan_x
                    ep_y = cy - (points[i+1][0] - self.center_lat) * 50000 * self.zoom + self.pan_y
                    painter.drawLine(int(sp_x), int(sp_y), int(ep_x), int(ep_y))

        # Compass rose
        painter.setPen(QColor(COLORS["text_secondary"]))
        painter.setFont(QFont("Consolas", 9))
        painter.drawText(cx - 4, 20, "N")
        painter.drawText(w - 20, cy + 4, "E")
        painter.drawText(cx - 4, h - 8, "S")
        painter.drawText(10, cy + 4, "W")

        # Center marker
        painter.setPen(QPen(QColor(COLORS["accent_cyan"]), 2))
        painter.drawLine(cx - 10, cy, cx + 10, cy)
        painter.drawLine(cx, cy - 10, cx, cy + 10)

        # Legend
        painter.setPen(QColor(COLORS["text_muted"]))
        painter.setFont(QFont("Consolas", 8))
        legend_y = 30
        for tl in [ThreatLevel.HIGH, ThreatLevel.MEDIUM, ThreatLevel.LOW, ThreatLevel.NONE]:
            painter.setPen(QPen(QColor(tl.color), 2))
            painter.drawRect(10, legend_y, 8, 8)
            painter.setPen(QColor(COLORS["text_muted"]))
            painter.drawText(22, legend_y + 8, tl.name.title())
            legend_y += 16

        painter.end()