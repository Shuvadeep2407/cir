"""
MED-FLIGHT Counter-UAS Detection System
Alert model
"""
from dataclasses import dataclass, field
from datetime import datetime
from uuid import uuid4
from utils.constants import AlertSeverity


@dataclass
class AlertModel:
    """System alert/event notification"""
    alert_id: str = field(default_factory=lambda: uuid4().hex[:12].upper())
    timestamp: datetime = field(default_factory=datetime.now)
    severity: AlertSeverity = AlertSeverity.INFO
    source: str = "SYSTEM"
    title: str = ""
    message: str = ""
    target_id: str = ""
    acknowledged: bool = False
    acknowledged_by: str = ""
    acknowledged_at: datetime = None

    def acknowledge(self, username: str = "OPERATOR"):
        self.acknowledged = True
        self.acknowledged_by = username
        self.acknowledged_at = datetime.now()

    def to_dict(self) -> dict:
        return {
            "alert_id": self.alert_id,
            "timestamp": self.timestamp.isoformat(),
            "severity": self.severity.label,
            "severity_color": self.severity.color,
            "source": self.source,
            "title": self.title,
            "message": self.message,
            "target_id": self.target_id,
            "acknowledged": self.acknowledged,
            "acknowledged_by": self.acknowledged_by,
        }