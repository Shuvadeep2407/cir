"""
MED-FLIGHT Counter-UAS Detection System
User model and session management
"""
from dataclasses import dataclass, field
from datetime import datetime
from typing import Optional
from hashlib import sha256
from uuid import uuid4


@dataclass
class UserModel:
    """System operator/user account"""
    user_id: str = field(default_factory=lambda: uuid4().hex[:8].upper())
    username: str = ""
    _password_hash: str = ""
    full_name: str = ""
    role: str = "OPERATOR"  # ADMIN, OPERATOR, ANALYST, MAINTENANCE
    email: str = ""
    active: bool = True
    created_at: datetime = field(default_factory=datetime.now)
    last_login: Optional[datetime] = None
    permissions: list = field(default_factory=lambda: ["view_live", "view_playback"])

    def set_password(self, password: str):
        self._password_hash = sha256(password.encode()).hexdigest()

    def verify_password(self, password: str) -> bool:
        return sha256(password.encode()).hexdigest() == self._password_hash

    def to_dict(self) -> dict:
        return {
            "user_id": self.user_id,
            "username": self.username,
            "full_name": self.full_name,
            "role": self.role,
            "email": self.email,
            "active": self.active,
            "created_at": self.created_at.isoformat(),
            "last_login": self.last_login.isoformat() if self.last_login else None,
            "permissions": self.permissions,
        }


@dataclass
class UserSession:
    """Active user session"""
    session_id: str = field(default_factory=lambda: uuid4().hex)
    user: Optional[UserModel] = None
    login_time: datetime = field(default_factory=datetime.now)
    last_activity: datetime = field(default_factory=datetime.now)
    ip_address: str = "127.0.0.1"
    active: bool = True

    def touch(self):
        self.last_activity = datetime.now()

    @property
    def session_duration(self) -> float:
        return (datetime.now() - self.login_time).total_seconds()