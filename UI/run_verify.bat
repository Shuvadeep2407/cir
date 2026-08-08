@echo off
cd /d e:\drone\UI
echo === Verifying Python Interpreter ===
.venv\Scripts\python.exe --version
echo.
echo === Checking PySide6 ===
.venv\Scripts\python.exe -c "import PySide6; print('PySide6:', PySide6.__version__)"
echo.
echo === Running MED-FLIGHT Application ===
.venv\Scripts\python.exe main.py
pause