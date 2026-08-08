"""
MED-FLIGHT Counter-UAS Detection System
Dummy sensor simulators that generate realistic synthetic data
"""
import math
import random
import numpy as np
from datetime import datetime
from utils.constants import SensorType
from models.sensor import SensorModel, SensorReading


class RadarSimulator:
    """Simulates radar returns with target detection"""
    def __init__(self):
        self.scan_angle = 0.0
        self.time = 0.0

    def generate_reading(self, sensor: SensorModel) -> SensorReading:
        self.time += 0.5
        self.scan_angle = (self.scan_angle + 1.5) % 360
        sensor.azimuth = self.scan_angle
        sensor.elevation = random.uniform(-5, 45)
        
        # Simulate detection events
        has_target = random.random() < 0.15
        if has_target:
            value = random.uniform(100, 5000)  # range in meters
            confidence = random.uniform(0.7, 0.99)
            metadata = {
                "rcs_db": random.uniform(-30, 0),
                "doppler_ms": random.uniform(0, 50),
                "scan_angle": round(self.scan_angle, 1),
                "detections": random.randint(1, 5),
            }
            sensor.status = "ACTIVE"
        else:
            value = 0.0
            confidence = 0.0
            metadata = {"scan_angle": round(self.scan_angle, 1), "detections": 0}
            sensor.status = "ACTIVE" if random.random() > 0.05 else "STANDBY"

        sensor.range_m = value if has_target else sensor.range_m * 0.99
        sensor.temperature_c = 35 + random.uniform(-2, 5)
        sensor.cpu_usage = random.uniform(20, 60)
        sensor.error_rate = random.uniform(0, 0.03)

        return SensorReading(
            sensor_id=sensor.sensor_id,
            sensor_type=SensorType.RADAR,
            timestamp=datetime.now(),
            value=value,
            unit="m",
            confidence=confidence,
            metadata=metadata,
        )


class RFSimulator:
    """Simulates RF spectrum scanning"""
    def __init__(self):
        self.freq = 2400.0  # MHz

    def generate_reading(self, sensor: SensorModel) -> SensorReading:
        self.freq += random.uniform(-10, 10)
        self.freq = max(100, min(6000, self.freq))
        sensor.frequency_hz = self.freq * 1e6

        # Simulate RF signal detection
        signal_strength = random.uniform(-120, -30)  # dBm
        has_signal = signal_strength > -90
        if has_signal:
            confidence = random.uniform(0.6, 0.95)
            metadata = {
                "frequency_mhz": round(self.freq, 1),
                "signal_strength_db": round(signal_strength, 1),
                "modulation": random.choice(["OFDM", "DSSS", "FHSS", "QPSK", "BPSK"]),
                "bandwidth_mhz": round(random.uniform(5, 40), 1),
            }
            sensor.bandwidth_mhz = metadata["bandwidth_mhz"]
        else:
            confidence = 0.1
            metadata = {"frequency_mhz": round(self.freq, 1), "signal_strength_db": round(signal_strength, 1)}

        sensor.status = "ACTIVE" if random.random() > 0.03 else "STANDBY"
        sensor.temperature_c = 38 + random.uniform(-2, 3)
        sensor.cpu_usage = random.uniform(30, 55)

        return SensorReading(
            sensor_id=sensor.sensor_id,
            sensor_type=SensorType.RF,
            timestamp=datetime.now(),
            value=signal_strength if has_signal else -120,
            unit="dBm",
            confidence=confidence,
            metadata=metadata,
        )


class AcousticSimulator:
    """Simulates acoustic array detection"""
    def __init__(self):
        self.base_level = 40.0  # dB baseline

    def generate_reading(self, sensor: SensorModel) -> SensorReading:
        # Simulate drone acoustic signature
        has_detection = random.random() < 0.12
        if has_detection:
            db_level = self.base_level + random.uniform(15, 35)
            confidence = random.uniform(0.65, 0.93)
            metadata = {
                "frequency_hz": random.choice([150, 200, 250, 300, 350, 400, 500]),
                "harmonic_ratio": round(random.uniform(0.3, 0.9), 2),
                "direction_deg": round(random.uniform(0, 360), 1),
                "signature": random.choice(["QUADCOPTER", "HEXACOPTER", "FIXED_WING", "VTOL", "UNKNOWN"]),
            }
            sensor.status = "ACTIVE"
        else:
            db_level = self.base_level + random.uniform(-5, 8)
            confidence = max(0.0, random.uniform(0.0, 0.3))
            metadata = {"direction_deg": 0, "signature": "NONE"}

        sensor.temperature_c = 30 + random.uniform(-1, 2)
        sensor.cpu_usage = random.uniform(25, 50)
        sensor.error_rate = random.uniform(0, 0.02)

        return SensorReading(
            sensor_id=sensor.sensor_id,
            sensor_type=SensorType.ACOUSTIC,
            timestamp=datetime.now(),
            value=db_level,
            unit="dB",
            confidence=confidence,
            metadata=metadata,
        )


class LRFSensor:
    """Simulates Laser Rangefinder"""
    def __init__(self):
        self.range = 0.0

    def generate_reading(self, sensor: SensorModel) -> SensorReading:
        has_lock = random.random() < 0.2
        if has_lock:
            self.range = random.uniform(50, 3000)
            confidence = random.uniform(0.85, 0.99)
            metadata = {
                "azimuth": round(random.uniform(0, 360), 1),
                "elevation": round(random.uniform(-10, 60), 1),
                "snr": round(random.uniform(15, 50), 1),
            }
            sensor.azimuth = metadata["azimuth"]
            sensor.elevation = metadata["elevation"]
            sensor.range_m = self.range
            sensor.status = "ACTIVE"
        else:
            self.range = max(0, self.range - random.uniform(0, 10))
            confidence = 0.0
            metadata = {"azimuth": round(sensor.azimuth, 1), "elevation": round(sensor.elevation, 1)}
            sensor.status = "ACTIVE" if random.random() > 0.1 else "STANDBY"

        sensor.temperature_c = 32 + random.uniform(-1, 3)
        sensor.cpu_usage = random.uniform(15, 35)

        return SensorReading(
            sensor_id=sensor.sensor_id,
            sensor_type=SensorType.LRF,
            timestamp=datetime.now(),
            value=self.range,
            unit="m",
            confidence=confidence,
            metadata=metadata,
        )


class CameraSimulator:
    """Simulates camera frames with synthetic video content"""
    def __init__(self, mode: str = "EO"):
        self.mode = mode  # EO, IR, YOLO
        self.frame_count = 0
        self._last_frame = None

    def generate_reading(self, sensor: SensorModel) -> SensorReading:
        self.frame_count += 1
        sensor.cpu_usage = random.uniform(30, 75)
        sensor.temperature_c = 40 + random.uniform(-2, 5)

        # Generate synthetic frame metadata
        has_detection = random.random() < 0.08
        if has_detection:
            confidence = random.uniform(0.75, 0.98)
            metadata = {
                "mode": self.mode,
                "frame": self.frame_count,
                "detections": [
                    {
                        "bbox": [random.randint(100, 500), random.randint(100, 400),
                                 random.randint(50, 200), random.randint(50, 200)],
                        "class": random.choice(["DJI_MAVIC", "DJI_PHANTOM", "DJI_INSPIRE",
                                                "AUTEL_ROBOTICS", "UNKNOWN_UAS", "BIRD", "BALLOON"]),
                        "confidence": round(random.uniform(0.7, 0.99), 2),
                    }
                ],
                "resolution": "1920x1080",
                "fps": 30,
                "zoom": random.uniform(1.0, 10.0),
                "ir_temp": random.uniform(20, 45) if self.mode == "IR" else None,
            }
            sensor.status = "ACTIVE"
        else:
            confidence = 0.0
            metadata = {
                "mode": self.mode,
                "frame": self.frame_count,
                "detections": [],
                "resolution": "1920x1080",
                "fps": 30,
                "zoom": 1.0,
            }

        value = 1.0 if has_detection else 0.0
        return SensorReading(
            sensor_id=sensor.sensor_id,
            sensor_type=SensorType.YOLO if self.mode == "YOLO" else SensorType.CAMERA,
            timestamp=datetime.now(),
            value=value,
            unit="detections",
            confidence=confidence,
            metadata=metadata,
        )

    def get_frame(self) -> np.ndarray:
        """Generate synthetic camera frame as numpy array"""
        h, w = 480, 640
        if self.mode == "IR":
            # Thermal-like image
            frame = np.random.normal(30, 10, (h, w)).astype(np.uint8)
            frame = np.stack([frame] * 3, axis=-1)
        elif self.mode == "YOLO":
            frame = np.random.randint(50, 70, (h, w, 3), dtype=np.uint8)
        else:
            # EO camera with noise
            frame = np.random.randint(80, 160, (h, w, 3), dtype=np.uint8)

        # Add horizon line
        horizon = h // 2 + random.randint(-20, 20)
        frame[horizon:, :] = np.clip(frame[horizon:, :].astype(int) - 20, 0, 255).astype(np.uint8)

        # Add crosshair
        cx, cy = w // 2, h // 2
        frame[cy-10:cy+10, cx-1:cx+2] = [0, 255, 0]
        frame[cy-1:cy+2, cx-10:cx+10] = [0, 255, 0]

        return frame