"""
MED-FLIGHT Counter-UAS Detection System
Alert manager - handles system alerts and notifications
"""
from datetime import datetime
from PySide6.QtCore import QObject, Signal
from utils.logger import log
from utils.constants import AlertSeverity
from models.alert import AlertModel


class AlertManager(QObject):
    """Central alert management system"""
    alert_created = Signal(object)  # AlertModel
    alert_acknowledged = Signal(str, str)  # alert_id, username
    alerts_cleared = Signal()

    # Tkinter callback support
    alert_created_callback = None

    def __init__(self, parent=None):
        super().__init__(parent)
        self.alerts = []
        self.max_alerts = 500

    def create_alert(self, severity: AlertSeverity, source: str, title: str,
                     message: str, target_id: str = "") -> AlertModel:
        alert = AlertModel(
            severity=severity,
            source=source,
            title=title,
            message=message,
            target_id=target_id,
        )
        self.alerts.append(alert)
        if len(self.alerts) > self.max_alerts:
            self.alerts.pop(0)
        self.alert_created.emit(alert)
        if self.alert_created_callback:
            self.alert_created_callback(alert)
        log.warning(f"ALERT [{severity.label}] {source}: {title}")
        return alert

    def acknowledge_alert(self, alert_id: str, username: str = "OPERATOR"):
        for alert in self.alerts:
            if alert.alert_id == alert_id:
                alert.acknowledge(username)
                self.alert_acknowledged.emit(alert_id, username)
                break

    def get_active_alerts(self) -> list:
        return [a for a in self.alerts if not a.acknowledged]

    def get_unacknowledged_count(self) -> int:
        """Get count of unacknowledged critical+emergency alerts"""
        return len([a for a in self.alerts if not a.acknowledged and
                    a.severity in (AlertSeverity.CRITICAL, AlertSeverity.EMERGENCY)])

    def get_alerts_by_severity(self, severity: AlertSeverity) -> list:
        return [a for a in self.alerts if a.severity == severity]

    def clear_all(self):
        self.alerts.clear()
        self.alerts_cleared.emit()
        log.info("All alerts cleared")

    def create_system_alert(self, title: str, message: str):
        return self.create_alert(AlertSeverity.INFO, "SYSTEM", title, message)

    def create_warning(self, title: str, message: str, target_id: str = ""):
        return self.create_alert(AlertSeverity.WARNING, "SYSTEM", title, message, target_id)

    def create_critical(self, title: str, message: str, target_id: str = ""):
        return self.create_alert(AlertSeverity.CRITICAL, "SYSTEM", title, message, target_id)

    def create_emergency(self, title: str, message: str, target_id: str = ""):
        return self.create_alert(AlertSeverity.EMERGENCY, "SYSTEM", title, message, target_id)