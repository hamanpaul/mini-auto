# Gemini Project Overview: Mini-Auto

This document provides a high-level summary of the Mini-Auto project for quick onboarding and context retrieval.

## 1. Project Goal

The project is to build and control a small robotic vehicle ("Mini-Auto"). The system involves real-time hardware control, sensor data acquisition (thermal and visual), and a host application for high-level command and control via a web API.

## 2. System Architecture

The system is composed of three main components:

1.  **Arduino UNO (Primary Controller)**:
    *   **Role**: Manages all real-time hardware operations, including motors, LEDs, and buzzer. It reads sensor data from the thermal imager.
    *   **Communication**: Communicates with the Python host application via an ESP-01S Wi-Fi module over UART.
    *   **Source Code**: `src/miniauto/app_control.ino`

2.  **ESP32-S3 (Vision Module)**:
    *   **Role**: Functions as a dedicated camera module. It captures video and streams it over Wi-Fi.
    *   **Communication**: Communicates with the host application independently.
    *   **Source Code**: `src/miniauto/esp32_cam_stream_server.ino`

3.  **Python Host Application (Py Agent)**:
    *   **Role**: Provides a high-level interface for controlling the vehicle and retrieving data. It features a FastAPI server, processes camera streams with OpenCV, and handles thermal data analysis.
    *   **Entry Point**: `src/py_rear/main.py`
    *   **APIs**: Defined in `src/py_rear/apis/`

## 3. Key File Locations

- **Hardware Specification**: `docs/硬體規格書 (Hardware Specification Document)V3.txt`
- **Software Requirements**: `docs/軟體需求書 (Software Requirements Specification, SRS)-v2.txt`
- **Arduino Main Logic**: `src/miniauto/app_control.ino`
- **Python FastAPI Main**: `src/py_rear/main.py`
- **Python API Endpoints**: `src/py_rear/apis/`
- **Feature/Integration Tests**: `test/feature/*.sh`
- **Unit Tests**: `test/unit/api/*.py`
- **Pre-defined Test Commands**: `docs/test_commands.sh`

## 4. Development & Testing

- **Run Python Backend**: The FastAPI server can be started using an ASGI server like `uvicorn`. The target application is `src.py_rear.main:app`.
- **Run Tests**: The project contains shell scripts for feature testing (`test/feature/`) and Python scripts for unit testing (`test/unit/`). The `docs/test_commands.sh` file contains a set of commands for validation.
