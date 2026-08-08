"""
MED-FLIGHT Counter-UAS Detection System
Target/track data models
"""
from dataclasses import dataclass, field
from datetime import datetime
from typing import Optional
import numpy as np
from utils.constants import ThreatLevel


@dataclass
class TrackPoint:
    """A single track point in a target's history"""
    timestamp: datetime
    lat: float
    lon: float
    alt_m: float
    speed_ms: float
    heading_deg: float
    confidence: float = 1.0

    def to_dict(self) -> dict:
        return {
            "timestamp": self.timestamp.isoformat(),
            "lat": self.lat,
            "lon": self.lon,
            "alt_m": self.alt_m,
            "speed_ms": self.speed_ms,
            "heading_deg": self.heading_deg,
            "confidence": self.confidence,
        }


@dataclass
class TargetModel:
    """UAS target with track history and classification"""
    target_id: str
    first_seen: datetime
    last_seen: datetime
    lat: float = 0.0
    lon: float = 0.0
    alt_m: float = 0.0
    speed_ms: float = 0.0
    heading_deg: float = 0.0
    threat_level: ThreatLevel = ThreatLevel.NONE
    classification: str = "UNKNOWN"
    classification_confidence: float = 0.0
    track_points: list = field(default_factory=list)
    max_track_points: int = 120
    signature: str = ""
    rcs_db: float = -20.0
    rf_frequency_mhz: Optional[float] = None
    acoustic_profile: str = ""
    engaged: bool = False
    mitigated: bool = False
    notes: str = ""
    sensor_sources: list = field(default_factory=list)

    def update_position(self, lat: float, lon: float, alt_m: float, speed_ms: float, heading_deg: float):
        now = datetime.now()
        point = TrackPoint(now, lat, lon, alt_m, speed_ms, heading_deg)
        self.track_points.append(point)
        if len(self.track_points) > self.max_track_points:
            self.track_points.pop(0)
        self.lat = lat
        self.lon = lon
        self.alt_m = alt_m
        self.speed_ms = speed_ms
        self.heading_deg = heading_deg
        self.last_seen = now

    def get_track_array(self) -> np.ndarray:
        """Return track as numpy array for trajectory prediction"""
        if not self.track_points:
            return np.array([])
        return np.array([[p.lat, p.lon, p.alt_m, p.speed_ms, p.heading_deg] for p in self.track_points])

    def predict_trajectory(self, steps: int = 10) -> np.ndarray:
        """Simple linear trajectory prediction"""
        if len(self.track_points) < 3:
            return np.array([])
        recent = self.get_track_array()[-5:]
        if len(recent) < 2:
            return np.array([])
        dt_lat = np.mean(np.diff(recent[:, 0]))
        dt_lon = np.mean(np.diff(recent[:, 1]))
        dt_alt = np.mean(np.diff(recent[:, 2]))
        predictions = []
        current = [self.lat, self.lon, self.alt_m]
        for _ in range(steps):
            current[0] += dt_lat
            current[1] += dt_lon
            current[2] += dt_alt
            predictions.append(current.copy())
        return np.array(predictions)

    @property
    def age_seconds(self) -> float:
        return (datetime.now() - self.first_seen).total_seconds()

    @property
    def track_length(self) -> int:
        return len(self.track_points)

    @property
    def range_m(self) -> float:
        """Approximate range from origin (simplified)"""
        return np.sqrt(self.lat**2 + self.lon**2 + (self.alt_m / 1000)**2) * 111000

    def to_dict(self) -> dict:
        return {
            "target_id": self.target_id,
            "first_seen": self.first_seen.isoformat(),
            "last_seen": self.last_seen.isoformat(),
            "lat": round(self.lat, 6),
            "lon": round(self.lon, 6),
            "alt_m": round(self.alt_m, 1),
            "speed_ms": round(self.speed_ms, 1),
            "heading_deg": round(self.heading_deg, 1),
            "threat_level": self.threat_level.name,
            "classification": self.classification,
            "confidence": round(self.classification_confidence, 2),
            "track_length": self.track_length,
            "engaged": self.engaged,
            "mitigated": self.mitigated,
            "signature": self.signature,
            "rcs_db": round(self.rcs_db, 1),
        }