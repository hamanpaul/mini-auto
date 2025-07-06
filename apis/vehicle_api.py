from fastapi import APIRouter, Request
from pydantic import BaseModel
from typing import List, Optional

router = APIRouter()

# 全域變數，用於儲存最新的狀態和指令
latest_arduino_data = {"s": 0, "v": 0, "t": None, "i": None}
latest_command_sent = {"c": 0, "m": 0, "d": 0, "a": 0}
latest_esp32_cam_ip: Optional[str] = None # 新增：儲存最新的 ESP32-S3 IP

# 定義 /api/sync 請求的數據模型
class SyncRequest(BaseModel):
    s: int  # status_byte
    v: int  # voltage_mv
    t: Optional[List[List[int]]] = None  # thermal_matrix (optional)
    i: Optional[str] = None # esp32_ip (optional)

# 定義 /api/sync 回應的數據模型
class SyncResponse(BaseModel):
    c: int = 0  # command_byte
    m: int = 0  # motor_speed
    d: int = 0  # direction_angle
    a: int = 0  # servo_angle

# 定義 /api/register_camera 請求的數據模型
class RegisterCameraRequest(BaseModel):
    i: str # esp32_ip

@router.post("/api/sync", response_model=SyncResponse)
async def sync_data(request: Request, data: SyncRequest):
    global latest_arduino_data, latest_command_sent

    # 儲存最新收到的數據
    latest_arduino_data = data.model_dump()

    print(f"\n--- Received Sync Data ---")
    print(f"Status Byte (s): {data.s} (Binary: {bin(data.s)})")
    print(f"Voltage (v): {data.v} mV")
    if data.t:
        print(f"Thermal Matrix (t): {data.t}")
    if data.i:
        print(f"ESP32 IP (i): {data.i}")
    print(f"--------------------------")

    # --- 指令生成邏輯 (待實作) ---
    # 這裡將是根據收到的數據和應用邏輯，決定回傳什麼指令的地方。
    # 目前先回傳預設值 (車輛停止，無特殊指令)。
    response_command = SyncResponse()

    # 儲存最新發送的指令
    latest_command_sent = response_command.model_dump()

    return response_command

@router.post("/api/register_camera")
async def register_camera(request_data: RegisterCameraRequest):
    global latest_esp32_cam_ip
    latest_esp32_cam_ip = request_data.i
    print(f"\n--- Received ESP32-S3 IP Registration ---")
    print(f"Registered ESP32-S3 IP: {latest_esp32_cam_ip}")
    print(f"----------------------------------------")
    return {"message": "ESP32-S3 IP registered successfully"}

@router.get("/api/latest_data")
async def get_latest_data():
    return {"latest_data": latest_arduino_data, "latest_command": latest_command_sent, "esp32_cam_ip": latest_esp32_cam_ip}
