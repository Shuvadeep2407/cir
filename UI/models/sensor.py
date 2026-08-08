"""
MED-FLIGHT Counter-UAS Detection System
Sensor data models
"""
from dataclasses import dataclass, field
from datetime import datetime
from typing import Optional
import numpy as np
from utils.constants import SensorType


@dataclass
class SensorReading:
    """Single sensor reading with timestamp"""
    sensor_id: str
    sensor_type: SensorType
    timestamp: datetime
    value: float
    unit: str
    confidence: float = 1.0
    metadata: dict = field(default_factory=dict)

    def to_dict(self) -> dict:
        return {
            "sensor_id": self.sensor_id,
            "sensor_type": self.sensor_type.value,
            "timestamp": self.timestamp.isoformat(),
            "value": self.value,
            "unit": self.unit,
            "confidence": self.confidence,
            "metadata": self.metadata,
        }


@dataclass
class SensorModel:
    """Sensor device model with status and configuration"""
    sensor_id: str
    name: str
    sensor_type: SensorType
    status: str = "STANDBY"
    online: bool = False
    azimuth: float = 0.0
    elevation: float = 0.0
    range_m: float = 0.0
    frequency_hz: Optional[float] = None
    bandwidth_mhz: Optional[float] = None
    fov_deg: float = 60.0
    last_reading: Optional[SensorReading] = None
    readings: list = field(default_factory=list)
    max_readings: int = 1000
    error_rate: float = 0.0
    firmware_version: str = "2.4.0"
    temperature_c: float = 35.0
    cpu_usage: float = 0.0

    def add_reading(self, reading: SensorReading):
        self.readings.append(reading)
        if len(self.readings) > self.max_readings:
            self.readings.pop(0)
        self.last_reading = reading

    def get_recent_readings(self, count: int = 10) -> list:
        return self.readings[-count:] if self.readings else []

    def get_value_history(self, count: int = 100) -> np.ndarray:
        values = [r.value for r in self.readings[-count:]]
        return np.array(values) if values else np.array([])

    def to_dict(self) -> dict:
        return {
            "sensor_id": self.sensor_id,
            "name": self.name,
            "type": self.sensor_type.value,
            "status": self.status,
            "online": self.online,
            "azimuth": round(self.azimuth, 1),
            "elevation": round(self.elevation, 1),
            "range_m": round(self.range_m, 1),
            "frequency_hz": self.frequency_hz,
            "bandwidth_mhz": self.bandwidth_mhz,
            "fov_deg": self.fov_deg,
            "temperature_c": round(self.temperature_c, 1),
            "cpu_usage": round(self.cpu_usage, 1),
            "error_rate": round(self.error_rate, 3),
        }

    @property
    def status_color(self) -> str:
        colors = {
            "ACTIVE": "#00ff88",
            "STANDBY": "#ffcc00",
            "OFFLINE": "#ff2200",
            "ERROR": "#ff4400",
            "CALIBRATING": "#00aaff",
        }
        return colors.get(self.status, "#556677")