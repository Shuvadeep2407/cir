"""
MED-FLIGHT Counter-UAS Detection System
Constants and configuration values
"""
from enum import Enum, auto


class SystemStatus(Enum):
    STANDBY = "STANDBY"
    ACTIVE = "ACTIVE"
    DEGRADED = "DEGRADED"
    OFFLINE = "OFFLINE"
    CALIBRATING = "CALIBRATING"


class ThreatLevel(Enum):
    NONE = (0, "#00ff88")
    LOW = (1, "#ffcc00")
    MEDIUM = (2, "#ff8800")
    HIGH = (3, "#ff4400")
    CRITICAL = (4, "#ff0000")

    def __init__(self, level: int, color: str):
        self.level = level
        self.color = color


class SensorType(Enum):
    RADAR = "Radar"
    RF = "RF Scanner"
    ACOUSTIC = "Acoustic Array"
    LRF = "Laser Rangefinder"
    CAMERA = "EO/IR Camera"
    YOLO = "YOLO AI Vision"


class AlertSeverity(Enum):
    INFO = ("INFO", "#00ccff")
    WARNING = ("WARNING", "#ffcc00")
    CRITICAL = ("CRITICAL", "#ff4400")
    EMERGENCY = ("EMERGENCY", "#ff0000")

    def __init__(self, label: str, color: str):
        self.label = label
        self.color = color


class MissionPhase(Enum):
    INITIALIZING = "INITIALIZING"
    SCANNING = "SCANNING"
    TRACKING = "TRACKING"
    ENGAGING = "ENGAGING"
    COMPLETED = "COMPLETED"
    ABORTED = "ABORTED"


# Application Constants
APP_NAME = "MED-FLIGHT C-UAS Detection System"
APP_VERSION = "2.4.0"
APP_BUILD = "2026.06.24"
COMPANY_NAME = "MED-FLIGHT Defense Systems"

# UI Constants
WINDOW_MIN_WIDTH = 1600
WINDOW_MIN_HEIGHT = 900
SIDEBAR_WIDTH = 220
TOP_BAR_HEIGHT = 48
ANIMATION_DURATION = 250

# Color Palette - Military Dark Theme
COLORS = {
    "bg_dark": "#0a0e14",
    "bg_primary": "#11161e",
    "bg_secondary": "#1a2030",
    "bg_tertiary": "#232b3d",
    "bg_card": "#1a2235",
    "bg_input": "#0d1219",
    "border": "#2a3550",
    "border_active": "#00aaff",
    "text_primary": "#e8edf5",
    "text_secondary": "#8899bb",
    "text_muted": "#556677",
    "accent_cyan": "#00ccff",
    "accent_blue": "#0066ff",
    "accent_green": "#00ff88",
    "accent_yellow": "#ffcc00",
    "accent_orange": "#ff8800",
    "accent_red": "#ff2200",
    "accent_purple": "#8800ff",
    "success": "#00ff66",
    "warning": "#ffaa00",
    "danger": "#ff2200",
    "info": "#00aaff",
    "radar_color": "#00ff88",
    "rf_color": "#ffcc00",
    "acoustic_color": "#00aaff",
    "lrf_color": "#ff6600",
    "camera_color": "#aa66ff",
}

# Sensor Update Intervals (ms)
SENSOR_INTERVALS = {
    "RADAR": 500,
    "RF": 800,
    "ACOUSTIC": 600,
    "LRF": 1000,
    "CAMERA": 33,  # ~30fps
    "YOLO": 100,
}

# Map Defaults
DEFAULT_LAT = 34.0522
DEFAULT_LON = -118.2437
DEFAULT_ZOOM = 14
TRACK_HISTORY_LENGTH = 120

# Database
DB_PATH = "data/medflight.db"

# Logging
LOG_DIR = "logs"
LOG_MAX_BYTES = 10 * 1024 * 1024
LOG_BACKUP_COUNT = 7