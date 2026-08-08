"""
MED-FLIGHT Counter-UAS Detection System
Sensor manager - handles all sensor lifecycle
"""
import time
import json
import numpy as np
from threading import Thread, Event
from datetime import datetime
from PySide6.QtCore import QObject, Signal, QThread
from utils.logger import log
from utils.constants import SensorType, SENSOR_INTERVALS
from models.sensor import SensorModel, SensorReading
from models.target import TargetModel
from .simulators import RadarSimulator, RFSimulator, AcousticSimulator, LRFSensor, CameraSimulator


class SensorWorker(QThread):
    """Worker thread for sensor data generation"""
    reading_ready = Signal(object)  # SensorReading

    def __init__(self, sensor: SensorModel, simulator):
        super().__init__()
        self.sensor = sensor
        self.simulator = simulator
        self._running = Event()
        self._interval = SENSOR_INTERVALS.get(sensor.sensor_id.split("_")[0].upper(), 500) / 1000.0

    def run(self):
        self._running.set()
        log.info(f"Starting sensor worker: {self.sensor.name}")
        while self._running.is_set():
            try:
                reading = self.simulator.generate_reading(self.sensor)
                if reading:
                    reading.timestamp = datetime.now()
                    self.sensor.add_reading(reading)
                    self.reading_ready.emit(reading)
            except Exception as e:
                log.error(f"Sensor {self.sensor.name} error: {e}")
            time.sleep(self._interval)

    def stop(self):
        self._running.clear()
        log.info(f"Stopped sensor worker: {self.sensor.name}")


class SensorManager(QObject):
    """Manages all sensor devices and their data streams"""
    sensor_updated = Signal(str)  # sensor_id
    new_reading = Signal(object)  # SensorReading
    sensor_error = Signal(str, str)  # sensor_id, error_msg

    def __init__(self, parent=None):
        super().__init__(parent)
        self.sensors = {}
        self.workers = {}
        self._create_sensors()

    def _create_sensors(self):
        sims = {
            "RADAR_01": (SensorModel("RADAR_01", "AN/MPQ-64 Radar", SensorType.RADAR, online=True), RadarSimulator()),
            "RF_01": (SensorModel("RF_01", "RF-3000 Scanner", SensorType.RF, online=True), RFSimulator()),
            "ACOUSTIC_01": (SensorModel("ACOUSTIC_01", "Audi Array M2K", SensorType.ACOUSTIC, online=True), AcousticSimulator()),
            "LRF_01": (SensorModel("LRF_01", "LRF-2200 Rangefinder", SensorType.LRF, online=True), LRFSensor()),
            "CAM_EO_01": (SensorModel("CAM_EO_01", "EO/IR Camera-1", SensorType.CAMERA, online=True), CameraSimulator("EO")),
            "CAM_IR_01": (SensorModel("CAM_IR_01", "EO/IR Camera-2", SensorType.CAMERA, online=True), CameraSimulator("IR")),
            "CAM_EO_02": (SensorModel("CAM_EO_02", "EO/IR Camera-3", SensorType.CAMERA, online=True), CameraSimulator("EO")),
            "CAM_IR_02": (SensorModel("CAM_IR_02", "EO/IR Camera-4", SensorType.CAMERA, online=True), CameraSimulator("IR")),
            "YOLO_01": (SensorModel("YOLO_01", "YOLO AI Vision", SensorType.YOLO, online=True), CameraSimulator("YOLO")),
        }
        for sid, (sensor, sim) in sims.items():
            self.sensors[sid] = sensor
            log.info(f"Created sensor: {sensor.name} ({sensor.sensor_id})")

    def start_all(self):
        for sid, sensor in self.sensors.items():
            if sid not in self.workers:
                sim = self._get_simulator(sid)
                if sim:
                    worker = SensorWorker(sensor, sim)
                    worker.reading_ready.connect(lambda r, s=sid: self._on_reading(s, r))
                    self.workers[sid] = worker
                    worker.start()
        log.info(f"Started {len(self.workers)} sensor workers")

    def _get_simulator(self, sid: str):
        from .simulators import RadarSimulator, RFSimulator, AcousticSimulator, LRFSensor, CameraSimulator
        sim_map = {
            "RADAR_01": RadarSimulator(),
            "RF_01": RFSimulator(),
            "ACOUSTIC_01": AcousticSimulator(),
            "LRF_01": LRFSensor(),
            "CAM_EO_01": CameraSimulator("EO"),
            "CAM_IR_01": CameraSimulator("IR"),
            "CAM_EO_02": CameraSimulator("EO"),
            "CAM_IR_02": CameraSimulator("IR"),
            "YOLO_01": CameraSimulator("YOLO"),
        }
        return sim_map.get(sid)

    def _on_reading(self, sid: str, reading: SensorReading):
        self.sensor_updated.emit(sid)
        self.new_reading.emit(reading)

    def stop_all(self):
        for worker in self.workers.values():
            worker.stop()
        for worker in self.workers.values():
            worker.wait(2000)
        self.workers.clear()
        log.info("All sensor workers stopped")

    def get_sensor(self, sid: str) -> SensorModel:
        return self.sensors.get(sid)

    def get_all_sensors(self) -> list:
        return list(self.sensors.values())

    def get_sensors_by_type(self, stype: SensorType) -> list:
        return [s for s in self.sensors.values() if s.sensor_type == stype]

    def toggle_sensor(self, sid: str, enable: bool):
        sensor = self.sensors.get(sid)
        if sensor:
            if enable and sid not in self.workers:
                sim = self._get_simulator(sid)
                if sim:
                    worker = SensorWorker(sensor, sim)
                    worker.reading_ready.connect(lambda r: self._on_reading(sid, r))
                    self.workers[sid] = worker
                    worker.start()
                    sensor.online = True
            elif not enable and sid in self.workers:
                self.workers[sid].stop()
                self.workers[sid].wait(1000)
                del self.workers[sid]
                sensor.online = False
            self.sensor_updated.emit(sid)