"""
MED-FLIGHT Counter-UAS Detection System
Target manager - tracks and manages UAS targets
"""
import uuid
import random
import numpy as np
from datetime import datetime
from PySide6.QtCore import QObject, Signal, QTimer
from utils.logger import log
from utils.constants import ThreatLevel
from models.target import TargetModel


class TargetManager(QObject):
    """Manages detected targets and track data"""
    target_added = Signal(object)  # TargetModel
    target_updated = Signal(object)  # TargetModel
    target_lost = Signal(str)  # target_id
    threat_assessment = Signal(str, str)  # target_id, threat_level

    # Tkinter callback support
    target_added_callback = None
    target_updated_callback = None

    def __init__(self, parent=None):
        super().__init__(parent)
        self.targets = {}
        self._id_counter = 0
        self._simulate_existing_targets()
        self._update_timer = QTimer(self)
        self._update_timer.timeout.connect(self._simulate_movement)
        self._update_timer.start(1000)

    def _simulate_existing_targets(self):
        """Create some initial simulated targets for demo"""
        initial_targets = [
            {"lat": 34.058, "lon": -118.248, "alt": 150, "speed": 12.5, "heading": 45,
             "classification": "DJI_MAVIC_3", "threat": ThreatLevel.LOW},
            {"lat": 34.048, "lon": -118.235, "alt": 300, "speed": 18.2, "heading": 270,
             "classification": "DJI_PHANTOM_4", "threat": ThreatLevel.MEDIUM},
            {"lat": 34.055, "lon": -118.240, "alt": 500, "speed": 8.0, "heading": 180,
             "classification": "AUTEL_ROBOTICS", "threat": ThreatLevel.LOW},
        ]
        for t in initial_targets:
            self._create_target(t["lat"], t["lon"], t["alt"], t["speed"],
                               t["heading"], t["classification"], t["threat"])

    def _create_target(self, lat, lon, alt, speed, heading, classification, threat):
        tid = f"UAS-{self._id_counter:04d}"
        self._id_counter += 1
        now = datetime.now()
        target = TargetModel(
            target_id=tid,
            first_seen=now,
            last_seen=now,
            lat=lat,
            lon=lon,
            alt_m=alt,
            speed_ms=speed,
            heading_deg=heading,
            threat_level=threat,
            classification=classification,
            classification_confidence=random.uniform(0.75, 0.98),
            rcs_db=random.uniform(-25, -5),
        )
        self.targets[tid] = target
        self.target_added.emit(target)
        if self.target_added_callback:
            self.target_added_callback(target)
        log.info(f"New target detected: {tid} ({classification})")
        return target

    def _simulate_movement(self):
        """Update target positions for demo"""
        for tid, target in self.targets.items():
            heading_rad = np.radians(target.heading_deg)
            speed_factor = target.speed_ms * 0.001

            new_lat = target.lat + speed_factor * np.cos(heading_rad) + random.uniform(-0.0005, 0.0005)
            new_lon = target.lon + speed_factor * np.sin(heading_rad) + random.uniform(-0.0005, 0.0005)
            new_alt = target.alt_m + random.uniform(-5, 5)
            new_alt = max(10, min(1500, new_alt))
            new_speed = max(0, target.speed_ms + random.uniform(-1, 1))
            new_heading = (target.heading_deg + random.uniform(-5, 5)) % 360

            target.update_position(new_lat, new_lon, new_alt, new_speed, new_heading)

            if target.alt_m < 100 and target.speed_ms > 15:
                target.threat_level = ThreatLevel.HIGH
            elif target.alt_m < 200:
                target.threat_level = ThreatLevel.MEDIUM
            elif target.alt_m > 500:
                target.threat_level = ThreatLevel.LOW
            else:
                target.threat_level = ThreatLevel.NONE

            self.target_updated.emit(target)
            if self.target_updated_callback:
                self.target_updated_callback(target)

        if random.random() < 0.02 and len(self.targets) < 8:
            lat = 34.05 + random.uniform(-0.03, 0.03)
            lon = -118.24 + random.uniform(-0.03, 0.03)
            alt = random.uniform(50, 800)
            speed = random.uniform(5, 25)
            heading = random.uniform(0, 360)
            classification = random.choice(["DJI_MAVIC_3", "DJI_PHANTOM_4", "DJI_INSPIRE",
                                           "AUTEL_ROBOTICS", "SKYDIO", "UNKNOWN_UAS"])
            threat = random.choice([ThreatLevel.LOW, ThreatLevel.MEDIUM])
            self._create_target(lat, lon, alt, speed, heading, classification, threat)

    def get_target(self, target_id: str) -> TargetModel:
        return self.targets.get(target_id)

    def get_all_targets(self) -> list:
        return list(self.targets.values())

    def get_active_targets(self) -> list:
        return [t for t in self.targets.values()
                if (datetime.now() - t.last_seen).total_seconds() < 30]

    def get_threat_summary(self) -> dict:
        summary = {"NONE": 0, "LOW": 0, "MEDIUM": 0, "HIGH": 0, "CRITICAL": 0}
        for t in self.targets.values():
            summary[t.threat_level.name] += 1
        return summary

    def remove_target(self, target_id: str):
        if target_id in self.targets:
            del self.targets[target_id]
            self.target_lost.emit(target_id)
            log.info(f"Target lost: {target_id}")

    def clear_all(self):
        self.targets.clear()
        log.info("All targets cleared")