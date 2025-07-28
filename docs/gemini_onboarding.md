# Gemini Project Overview: Mini-Auto (Post-Refactoring)

This document provides a high-level summary of the Mini-Auto project after the architectural refactoring.

## 1. Project Goal

The project is to build and control a small robotic vehicle ("Mini-Auto"). The system involves real-time hardware control, sensor data acquisition (thermal, ultrasonic, and visual), and a host application for high-level command and control via a web API.

## 2. System Architecture

The system is composed of three main components with a clear separation of concerns:

1.  **Arduino UNO (Real-time Controller)**:
    *   **Role**: Manages all real-time hardware operations, including motors, LEDs, buzzer, and reading sensors (thermal, ultrasonic, voltage).
    *   **Communication**: Communicates **only** with the ESP32-S3 via I2C. It sends sensor data and receives control commands.
    *   **Source Code**: `src/miniauto/arduino_uno/arduino_uno.ino`

2.  **ESP32-S3 (Vision Module & Network Agent)**:
    *   **Role**: Functions as a dedicated camera module and the sole network agent for the vehicle.
        *   **Vision**: Captures and streams video over Wi-Fi.
        *   **Network**: Handles all Wi-Fi communication with the Python host, including service discovery (UDP), data synchronization (`/api/sync`), and camera registration (`/api/register_camera`).
    *   **Communication**: Acts as a bridge. It communicates with the Arduino UNO via I2C (as a slave) and with the Python host application via Wi-Fi (as an HTTP client).
    *   **Source Code**: `src/miniauto/esp32_cam/esp32_cam.ino`

3.  **Python Host Application (Py Agent)**:
    *   **Role**: The central brain providing high-level control. It features a FastAPI server, processes camera streams with OpenCV, analyzes sensor data, and makes control decisions.
    *   **Entry Point**: `main.py`
    *   **APIs**: Defined in `src/py_rear/apis/`

## 3. Key File Locations

- **Hardware Specification**: `docs/硬體規格書 (Hardware Specification Document)V3.txt`
- **Software Requirements**: `docs/軟體需求書 (Software Requirements Specification, SRS)-v2.txt`
- **Arduino Main Logic**: `src/miniauto/arduino_uno/arduino_uno.ino`
- **ESP32 Main Logic**: `src/miniauto/esp32_cam/esp32_cam.ino`
- **Python FastAPI Main**: `main.py`
- **Python API Endpoints**: `src/py_rear/apis/`
- **Feature/Integration Tests**: `test/feature/*.sh`
- **Unit Tests**: `test/unit/api/*.py`
- **Pre-defined Test Commands**: `docs/test_commands.sh`

## 4. Development & Testing

- **Run Python Backend**: The FastAPI server can be started using `python main.py`.
- **Run Tests**: The project contains shell scripts for feature testing (`test/feature/`) and Python scripts for unit testing (`test/unit/`). The `docs/test_commands.sh` file contains a set of commands for validation.
