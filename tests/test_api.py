import pytest
from httpx import AsyncClient
from main import app
from apis.vehicle_api import latest_arduino_data, latest_command_sent, latest_esp32_cam_ip, current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte, current_control_mode

@pytest.mark.asyncio
async def test_sync_api():
    async with AsyncClient(app=app, base_url="http://test") as client:
        # Test with thermal data
        response = await client.post(
            "/api/sync",
            json={
                "s": 79,  # Example status byte
                "v": 785, # Example voltage
                "t": [[2550, 2600], [2575, 2625]] # Example thermal data
            },
        )
        assert response.status_code == 200
        assert "c" in response.json()
        assert "m" in response.json()
        assert "d" in response.json()
        assert "a" in response.json()
        assert latest_arduino_data["s"] == 79
        assert latest_arduino_data["v"] == 785
        assert latest_arduino_data["t"] == [[2550, 2600], [2575, 2625]]

        # Test without thermal data
        response = await client.post(
            "/api/sync",
            json={
                "s": 78,  # Different status byte
                "v": 750
            },
        )
        assert response.status_code == 200
        assert latest_arduino_data["s"] == 78
        assert latest_arduino_data["v"] == 750
        assert latest_arduino_data["t"] is None

@pytest.mark.asyncio
async def test_register_camera_api():
    async with AsyncClient(app=app, base_url="http://test") as client:
        test_ip = "192.168.1.100"
        response = await client.post(
            "/api/register_camera",
            json={
                "i": test_ip
            },
        )
        assert response.status_code == 200
        assert response.json() == {"message": "ESP32-S3 IP registered successfully"}
        assert latest_esp32_cam_ip == test_ip

@pytest.mark.asyncio
async def test_manual_control_api():
    async with AsyncClient(app=app, base_url="http://test") as client:
        # Set some manual control values
        test_m = 50
        test_d = 90
        test_a = 45
        test_c = 1 # Example command byte

        response = await client.post(
            "/api/manual_control",
            json={
                "m": test_m,
                "d": test_d,
                "a": test_a,
                "c": test_c
            },
        )
        assert response.status_code == 200
        assert response.json() == {"message": "Manual control command updated"}
        assert current_manual_motor_speed == test_m
        assert current_manual_direction_angle == test_d
        assert current_manual_servo_angle == test_a
        assert current_manual_command_byte == test_c

@pytest.mark.asyncio
async def test_set_control_mode_api():
    async with AsyncClient(app=app, base_url="http://test") as client:
        # Test setting to avoidance mode
        response = await client.post(
            "/api/set_control_mode",
            json={
                "mode": "avoidance"
            },
        )
        assert response.status_code == 200
        assert response.json() == {"message": "Control mode set to avoidance"}
        assert current_control_mode.value == "avoidance"

        # Test setting back to manual mode
        response = await client.post(
            "/api/set_control_mode",
            json={
                "mode": "manual"
            },
        )
        assert response.status_code == 200
        assert current_control_mode.value == "manual"

@pytest.mark.asyncio
async def test_latest_data_api():
    async with AsyncClient(app=app, base_url="http://test") as client:
        # First, send some sync data to populate latest_arduino_data
        await client.post(
            "/api/sync",
            json={
                "s": 100,
                "v": 800,
                "t": [[100, 200], [300, 400]],
                "i": "192.168.1.200"
            },
        )
        # Then, set some manual control data
        await client.post(
            "/api/manual_control",
            json={
                "m": 60,
                "d": 180,
                "a": 70,
                "c": 2
            },
        )
        # And set control mode
        await client.post(
            "/api/set_control_mode",
            json={
                "mode": "autonomous"
            },
        )

        response = await client.get("/api/latest_data")
        assert response.status_code == 200
        data = response.json()

        assert data["latest_data"]["s"] == 100
        assert data["latest_data"]["v"] == 800
        assert data["latest_data"]["t"] == [[100, 200], [300, 400]]
        assert data["latest_data"]["i"] == "192.168.1.200"

        assert data["latest_command"]["m"] == 60
        assert data["latest_command"]["d"] == 180
        assert data["latest_command"]["a"] == 70
        assert data["latest_command"]["c"] == 2

        assert data["esp32_cam_ip"] == "192.168.1.200"
        assert data["current_control_mode"] == "autonomous"
