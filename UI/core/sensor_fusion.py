"""
MED-FLIGHT Counter-UAS Detection System
Sensor fusion engine - combines multiple sensor inputs
"""
import numpy as np
from datetime import datetime
from PySide6.QtCore import QObject, Signal
from utils.logger import log
from utils.constants import SensorType, ThreatLevel
from models.sensor import SensorReading
from models.target import TargetModel


class SensorFusionEngine(QObject):
    """Fuses data from all sensors for improved detection"""
    fused_detection = Signal(object)  # dict with fused target data

    def __init__(self, parent=None):
        super().__init__(parent)
        self.fusion_weight = {
            SensorType.RADAR: 0.35,
            SensorType.RF: 0.25,
            SensorType.ACOUSTIC: 0.15,
            SensorType.CAMERA: 0.15,
            SensorType.LRF: 0.10,
        }
        self.recent_readings = {st: [] for st in SensorType}
        self.max_readings_per_type = 50

    def process_reading(self, reading: SensorReading):
        """Process a sensor reading through fusion"""
        stype = reading.sensor_type
        self.recent_readings[stype].append(reading)
        if len(self.recent_readings[stype]) > self.max_readings_per_type:
            self.recent_readings[stype].pop(0)

        # Check for fusion opportunities
        if self._should_fuse():
            result = self._fuse_readings()
            if result:
                self.fused_detection.emit(result)

    def _should_fuse(self) -> bool:
        """Check if we have enough data from multiple sensors to fuse"""
        active_types = sum(1 for st, readings in self.recent_readings.items()
                          if readings and readings[-1].confidence > 0.3)
        return active_types >= 2

    def _fuse_readings(self) -> dict:
        """Fuse recent readings from all sensors"""
        positions = []
        total_weight = 0

        for stype, readings in self.recent_readings.items():
            if not readings:
                continue
            latest = readings[-1]
            if latest.confidence < 0.3:
                continue

            weight = self.fusion_weight.get(stype, 0.1)
            metadata = latest.metadata

            # Extract position estimate based on sensor type
            pos = self._extract_position(stype, latest, metadata)
            if pos:
                positions.append((pos, weight * latest.confidence))
                total_weight += weight * latest.confidence

        if not positions or total_weight < 0.2:
            return None

        # Weighted average position
        fused_lat = sum(p[0][0] * p[1] for p in positions) / total_weight
        fused_lon = sum(p[0][1] * p[1] for p in positions) / total_weight
        fused_alt = sum(p[0][2] * p[1] for p in positions) / total_weight
        fused_confidence = min(1.0, total_weight * 1.5)

        # Determine fusion quality
        fusion_sources = [p[0][3] for p in positions]
        quality = "HIGH" if len(positions) >= 3 else "MEDIUM" if len(positions) >= 2 else "LOW"

        result = {
            "lat": fused_lat,
            "lon": fused_lon,
            "alt_m": fused_alt,
            "confidence": fused_confidence,
            "sources": fusion_sources,
            "fusion_quality": quality,
            "timestamp": datetime.now(),
            "sensor_count": len(positions),
        }
        return result

    def _extract_position(self, stype: SensorType, reading: SensorReading,
                          metadata: dict) -> tuple:
        """Extract position from sensor reading type"""
        if stype == SensorType.RADAR:
            # Radar gives range and bearing
            rng = reading.value
            if rng > 0:
                az = metadata.get("scan_angle", 0)
                el = metadata.get("elevation", 0)
                offset_lat = (rng / 111000) * np.cos(np.radians(az))
                offset_lon = (rng / (111000 * np.cos(np.radians(34.05)))) * np.sin(np.radians(az))
                alt = rng * np.sin(np.radians(el))
                return (34.05 + offset_lat, -118.24 + offset_lon, max(0, alt), "RADAR")

        elif stype == SensorType.RF:
            # RF gives signal detection direction
            freq = metadata.get("frequency_mhz", 0)
            if freq > 0:
                return (34.05 + np.random.uniform(-0.01, 0.01),
                       -118.24 + np.random.uniform(-0.01, 0.01),
                       np.random.uniform(100, 500), "RF")

        elif stype == SensorType.ACOUSTIC:
            # Acoustic gives direction
            if reading.confidence > 0.5:
                return (34.05 + np.random.uniform(-0.008, 0.008),
                       -118.24 + np.random.uniform(-0.008, 0.008),
                       np.random.uniform(50, 400), "ACOUSTIC")

        elif stype in (SensorType.CAMERA, SensorType.LRF):
            if reading.value > 0:
                return (34.05 + np.random.uniform(-0.005, 0.005),
                       -118.24 + np.random.uniform(-0.005, 0.005),
                       np.random.uniform(50, 300), stype.value)

        return None

    def get_fusion_weights(self) -> dict:
        return self.fusion_weight

    def update_weight(self, stype: SensorType, weight: float):
        if stype in self.fusion_weight:
            self.fusion_weight[stype] = max(0.0, min(1.0, weight))