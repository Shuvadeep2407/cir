"""
MED-FLIGHT Counter-UAS Detection System
SQLite database manager with schema management
"""
import sqlite3
from pathlib import Path
from datetime import datetime
from threading import Lock
from utils.logger import log
from utils.constants import DB_PATH


class DatabaseManager:
    """Thread-safe SQLite database manager"""
    _instance = None
    _lock = Lock()

    def __new__(cls):
        with cls._lock:
            if cls._instance is None:
                cls._instance = super().__new__(cls)
                cls._instance._initialized = False
            return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self._initialized = True
        self.db_path = Path(DB_PATH)
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        self._conn_lock = Lock()
        self._connection = None
        self._connect()
        self._create_schema()
        self._seed_data()
        log.info(f"Database initialized at {self.db_path}")

    def _connect(self):
        self._connection = sqlite3.connect(str(self.db_path), check_same_thread=False)
        self._connection.row_factory = sqlite3.Row
        self._connection.execute("PRAGMA journal_mode=WAL")
        self._connection.execute("PRAGMA foreign_keys=ON")

    def get_connection(self) -> sqlite3.Connection:
        with self._conn_lock:
            if self._connection is None:
                self._connect()
            return self._connection

    def _create_schema(self):
        conn = self.get_connection()
        cursor = conn.cursor()
        
        cursor.executescript("""
            CREATE TABLE IF NOT EXISTS users (
                user_id TEXT PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                full_name TEXT,
                role TEXT DEFAULT 'OPERATOR',
                email TEXT,
                active INTEGER DEFAULT 1,
                created_at TEXT,
                last_login TEXT,
                permissions TEXT DEFAULT '["view_live","view_playback"]'
            );

            CREATE TABLE IF NOT EXISTS sessions (
                session_id TEXT PRIMARY KEY,
                user_id TEXT,
                login_time TEXT,
                last_activity TEXT,
                ip_address TEXT,
                active INTEGER DEFAULT 1,
                FOREIGN KEY (user_id) REFERENCES users(user_id)
            );

            CREATE TABLE IF NOT EXISTS targets (
                target_id TEXT PRIMARY KEY,
                first_seen TEXT,
                last_seen TEXT,
                lat REAL,
                lon REAL,
                alt_m REAL,
                speed_ms REAL,
                heading_deg REAL,
                threat_level TEXT,
                classification TEXT,
                confidence REAL,
                signature TEXT,
                rcs_db REAL,
                rf_frequency_mhz REAL,
                acoustic_profile TEXT,
                engaged INTEGER DEFAULT 0,
                mitigated INTEGER DEFAULT 0,
                notes TEXT,
                mission_id TEXT
            );

            CREATE TABLE IF NOT EXISTS track_points (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                target_id TEXT,
                timestamp TEXT,
                lat REAL,
                lon REAL,
                alt_m REAL,
                speed_ms REAL,
                heading_deg REAL,
                confidence REAL,
                FOREIGN KEY (target_id) REFERENCES targets(target_id)
            );

            CREATE TABLE IF NOT EXISTS alerts (
                alert_id TEXT PRIMARY KEY,
                timestamp TEXT,
                severity TEXT,
                source TEXT,
                title TEXT,
                message TEXT,
                target_id TEXT,
                acknowledged INTEGER DEFAULT 0,
                acknowledged_by TEXT,
                acknowledged_at TEXT
            );

            CREATE TABLE IF NOT EXISTS missions (
                mission_id TEXT PRIMARY KEY,
                name TEXT,
                start_time TEXT,
                end_time TEXT,
                status TEXT,
                operator TEXT,
                location TEXT,
                notes TEXT,
                targets_count INTEGER DEFAULT 0
            );

            CREATE TABLE IF NOT EXISTS sensor_logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                sensor_id TEXT,
                timestamp TEXT,
                sensor_type TEXT,
                value REAL,
                unit TEXT,
                confidence REAL,
                metadata TEXT
            );

            CREATE TABLE IF NOT EXISTS system_events (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                event_type TEXT,
                source TEXT,
                description TEXT,
                data TEXT
            );

            CREATE TABLE IF NOT EXISTS recordings (
                recording_id TEXT PRIMARY KEY,
                mission_id TEXT,
                start_time TEXT,
                end_time TEXT,
                duration_seconds REAL,
                file_path TEXT,
                size_bytes INTEGER,
                status TEXT,
                notes TEXT
            );

            CREATE INDEX IF NOT EXISTS idx_track_target ON track_points(target_id);
            CREATE INDEX IF NOT EXISTS idx_alerts_timestamp ON alerts(timestamp);
            CREATE INDEX IF NOT EXISTS idx_sensor_logs_time ON sensor_logs(timestamp);
            CREATE INDEX IF NOT EXISTS idx_events_time ON system_events(timestamp);
        """)
        conn.commit()
        log.info("Database schema created/verified")

    def _seed_data(self):
        """Seed default admin user and sample data if empty"""
        conn = self.get_connection()
        cursor = conn.cursor()
        
        # Create default admin
        cursor.execute("SELECT COUNT(*) FROM users")
        if cursor.fetchone()[0] == 0:
            from hashlib import sha256
            admin_hash = sha256("admin123".encode()).hexdigest()
            cursor.execute(
                "INSERT INTO users (user_id, username, password_hash, full_name, role, active, created_at, permissions) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                ("ADMIN01", "admin", admin_hash, "System Administrator", "ADMIN", 1,
                 datetime.now().isoformat(), '["all"]')
            )
            operator_hash = sha256("operator123".encode()).hexdigest()
            cursor.execute(
                "INSERT INTO users (user_id, username, password_hash, full_name, role, active, created_at) "
                "VALUES (?, ?, ?, ?, ?, ?, ?)",
                ("OP001", "operator", operator_hash, "Mission Operator", "OPERATOR", 1,
                 datetime.now().isoformat())
            )
            conn.commit()
            log.info("Default users seeded")

    def execute(self, query: str, params: tuple = ()) -> sqlite3.Cursor:
        conn = self.get_connection()
        with self._conn_lock:
            cursor = conn.execute(query, params)
            conn.commit()
            return cursor

    def fetch_one(self, query: str, params: tuple = ()) -> dict:
        cursor = self.execute(query, params)
        row = cursor.fetchone()
        return dict(row) if row else None

    def fetch_all(self, query: str, params: tuple = ()) -> list:
        cursor = self.execute(query, params)
        return [dict(row) for row in cursor.fetchall()]

    def insert(self, table: str, data: dict) -> int:
        columns = ", ".join(data.keys())
        placeholders = ", ".join("?" * len(data))
        query = f"INSERT INTO {table} ({columns}) VALUES ({placeholders})"
        cursor = self.execute(query, tuple(data.values()))
        return cursor.lastrowid

    def update(self, table: str, data: dict, where: str, where_params: tuple = ()) -> int:
        sets = ", ".join(f"{k}=?" for k in data.keys())
        query = f"UPDATE {table} SET {sets} WHERE {where}"
        cursor = self.execute(query, tuple(data.values()) + where_params)
        return cursor.rowcount

    def delete(self, table: str, where: str, params: tuple = ()) -> int:
        query = f"DELETE FROM {table} WHERE {where}"
        cursor = self.execute(query, params)
        return cursor.rowcount

    def close(self):
        with self._conn_lock:
            if self._connection:
                self._connection.close()
                self._connection = None
                log.info("Database connection closed")