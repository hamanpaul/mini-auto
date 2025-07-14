# test/feature/gui/test_gui_backend.py
import pytest
from fastapi.testclient import TestClient
from src.py_rear.main import app

client = TestClient(app)

def test_manual_control_api():
    """Test the /api/manual_control endpoint."""
    response = client.post(
        "/api/manual_control",
        json={
            "m": 150,  # motor_speed
            "d": 90,   # direction_angle
            "a": 90,   # servo_angle
            "c": 1     # command_byte
        }
    )
    assert response.status_code == 200
    assert response.json() == {"message": "Manual control command updated"}

def test_set_control_mode_api():
    """Test the /api/set_control_mode endpoint."""
    # Test setting to avoidance mode
    response = client.post(
        "/api/set_control_mode",
        json={
            "mode": "avoidance"
        }
    )
    assert response.status_code == 200
    assert response.json() == {"message": "Control mode set to avoidance"}

    # Test setting to manual mode
    response = client.post(
        "/api/set_control_mode",
        json={
            "mode": "manual"
        }
    )
    assert response.status_code == 200
    assert response.json() == {"message": "Control mode set to manual"}

def test_register_camera_api():
    """Test the /api/register_camera endpoint."""
    test_ip = "192.168.1.200"
    response = client.post(
        "/api/register_camera",
        json={
            "i": test_ip
        }
    )
    assert response.status_code == 200
    assert response.json() == {"message": "ESP32-S3 IP registered successfully"}

def test_latest_data_api():
    """Test the /api/latest_data endpoint."""
    response = client.get("/api/latest_data")
    assert response.status_code == 200
    # The content of latest_data will depend on previous API calls or initial state
    # We can assert on the structure or expected default values
    data = response.json()
    assert "latest_data" in data
    assert "latest_command" in data
    assert "esp32_cam_ip" in data
    assert "current_control_mode" in data
    assert "thermal_analysis" in data
    assert "visual_analysis" in data

    # Test with some initial sync data to ensure it reflects
    client.post(
        "/api/sync",
        json={
            "s": 79, "v": 785, "t": [[2550, 2600], [2575, 2625]], "i": "192.168.1.200", "u": 50
        }
    )
    response = client.get("/api/latest_data")
    assert response.status_code == 200
    data = response.json()
    assert data["latest_data"]["s"] == 79
    assert data["latest_data"]["v"] == 785
    assert data["latest_data"]["u"] == 50
    assert data["latest_data"]["i"] == "192.168.1.200"
    assert "thermal_analysis" in data
    assert data["thermal_analysis"]["hotspot_detected"] == False