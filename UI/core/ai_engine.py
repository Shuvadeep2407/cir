"""
MED-FLIGHT Counter-UAS Detection System
AI Engine for target classification, trajectory prediction, and threat assessment
"""
import numpy as np
from datetime import datetime
from PySide6.QtCore import QObject, Signal, QTimer
from utils.logger import log
from utils.constants import ThreatLevel
from models.target import TargetModel


class AIEngine(QObject):
    """AI processing engine for target analysis"""
    classification_ready = Signal(str, str, float)  # target_id, classification, confidence
    trajectory_predicted = Signal(str, object)  # target_id, numpy array of predicted points
    threat_updated = Signal(str, str)  # target_id, threat_level
    ai_metrics = Signal(dict)  # performance metrics

    def __init__(self, parent=None):
        super().__init__(parent)
        self.classification_labels = [
            "DJI_MAVIC_3", "DJI_PHANTOM_4", "DJI_INSPIRE",
            "AUTEL_ROBOTICS", "SKYDIO", "PARROT_ANAFI",
            "YUNEEC_H520", "FIXED_WING", "VTOL", "BIRD",
            "BALLOON", "UNKNOWN_UAS",
        ]
        self.processing_queue = []
        self.metrics = {
            "classifications": 0,
            "predictions": 0,
            "avg_confidence": 0.0,
            "processing_time_ms": 0.0,
            "false_positives": 0,
        }
        self._process_timer = QTimer(self)
        self._process_timer.timeout.connect(self._process_queue)
        self._process_timer.start(500)

    def classify_target(self, target: TargetModel):
        """Queue target for ML classification"""
        if target.target_id not in [t[0] for t in self.processing_queue]:
            self.processing_queue.append((target.target_id, "classify", target))

    def predict_trajectory(self, target: TargetModel):
        """Queue target for trajectory prediction"""
        if target.target_id not in [t[0] for t in self.processing_queue]:
            self.processing_queue.append((target.target_id, "predict", target))

    def _process_queue(self):
        if not self.processing_queue:
            return

        target_id, task, target = self.processing_queue.pop(0)

        if task == "classify":
            self._run_classification(target_id, target)
        elif task == "predict":
            self._run_prediction(target)

    def _run_classification(self, target_id: str, target: TargetModel):
        """Simulate ML classification based on target features"""
        import time
        start = time.time()

        # Feature-based classification simulation
        speed = target.speed_ms
        alt = target.alt_m
        rcs = target.rcs_db

        scores = {}
        # DJI Mavic: slow-medium speed, low altitude
        scores["DJI_MAVIC_3"] = self._gaussian(speed, 10, 5) * self._gaussian(alt, 100, 50)
        # DJI Phantom: medium speed
        scores["DJI_PHANTOM_4"] = self._gaussian(speed, 15, 5) * self._gaussian(alt, 200, 80)
        # DJI Inspire: faster, higher
        scores["DJI_INSPIRE"] = self._gaussian(speed, 20, 6) * self._gaussian(alt, 300, 100)
        # Fixed wing: fast, high
        scores["FIXED_WING"] = self._gaussian(speed, 30, 10) * self._gaussian(alt, 500, 200)
        # Bird: slow, low altitude
        scores["BIRD"] = self._gaussian(speed, 8, 4) * self._gaussian(alt, 50, 40)

        # RCS contributes
        if -20 < rcs < -5:
            scores["DJI_MAVIC_3"] *= 1.2
        elif -15 < rcs < 0:
            scores["DJI_PHANTOM_4"] *= 1.3

        if not scores:
            classification = "UNKNOWN_UAS"
            confidence = 0.3
        else:
            best_label = max(scores, key=scores.get)
            best_score = scores[best_label]
            if best_score > 0.3:
                classification = best_label
                confidence = min(0.99, best_score)
            else:
                classification = "UNKNOWN_UAS"
                confidence = max(0.2, best_score)

        # Update target
        target.classification = classification
        target.classification_confidence = confidence

        # Update threat based on classification
        high_threat = ["DJI_INSPIRE", "FIXED_WING", "VTOL"]
        medium_threat = ["DJI_PHANTOM_4", "YUNEEC_H520", "SKYDIO"]
        if classification in high_threat:
            target.threat_level = ThreatLevel.HIGH
        elif classification in medium_threat:
            target.threat_level = ThreatLevel.MEDIUM
        elif classification == "BIRD":
            target.threat_level = ThreatLevel.NONE
        else:
            target.threat_level = ThreatLevel.LOW

        proc_time = (time.time() - start) * 1000
        self.metrics["classifications"] += 1
        self.metrics["avg_confidence"] = (self.metrics["avg_confidence"] * (
            self.metrics["classifications"] - 1) + confidence) / self.metrics["classifications"]
        self.metrics["processing_time_ms"] = proc_time

        self.classification_ready.emit(target_id, classification, confidence)
        self.threat_updated.emit(target_id, target.threat_level.name)
        self.ai_metrics.emit(self.metrics)

    def _run_prediction(self, target: TargetModel):
        """Run trajectory prediction"""
        import time
        start = time.time()

        predictions = target.predict_trajectory(steps=15)
        if len(predictions) > 0:
            self.metrics["predictions"] += 1
            self.metrics["processing_time_ms"] = (time.time() - start) * 1000
            self.trajectory_predicted.emit(target.target_id, predictions)
            self.ai_metrics.emit(self.metrics)

    def _gaussian(self, x: float, mu: float, sigma: float) -> float:
        return np.exp(-((x - mu) ** 2) / (2 * sigma ** 2))

    def run_yolo_detection(self, frame: np.ndarray) -> list:
        """Simulate YOLO detection on a frame"""
        detections = []
        if frame is not None:
            h, w = frame.shape[:2]
            # Simulated detections for demo
            for _ in range(int(np.random.poisson(0.3))):
                x, y = np.random.randint(0, w - 100), np.random.randint(0, h - 80)
                detections.append({
                    "bbox": [x, y, x + np.random.randint(40, 120), y + np.random.randint(30, 80)],
                    "class": np.random.choice(self.classification_labels[:6]),
                    "confidence": float(np.random.uniform(0.5, 0.98)),
                })
        return detections

    def reset_metrics(self):
        self.metrics = {
            "classifications": 0,
            "predictions": 0,
            "avg_confidence": 0.0,
            "processing_time_ms": 0.0,
            "false_positives": 0,
        }
        self.ai_metrics.emit(self.metrics)