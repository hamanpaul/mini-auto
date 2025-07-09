from fastapi import APIRouter, HTTPException, Body, Depends, Request
from pydantic import BaseModel
from enum import Enum
from typing import Optional, List

router = APIRouter()

# --- 控制模式定義 ---
class ControlMode(str, Enum):
    MANUAL = "manual"
    AVOIDANCE = "avoidance"
    AUTONOMOUS = "autonomous"

current_control_mode: ControlMode = ControlMode.MANUAL # 預設為手動控制模式

# 全域變數，用於儲存當前的手動控制指令
current_manual_motor_speed: int = 0
current_manual_direction_angle: int = 0
current_manual_servo_angle: int = 0
current_manual_command_byte: int = 0

# 全域變數，用於儲存最新收到的數據和發送的指令
latest_arduino_data: Optional[dict] = None
latest_command_sent: Optional[dict] = None
latest_esp32_cam_ip: Optional[str] = None
latest_thermal_analysis_results: Optional[dict] = None
camera_processor_instance = None # CameraStreamProcessor instance

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

# 定義 /api/manual_control 請求的數據模型
class ManualControlRequest(BaseModel):
    m: int = 0  # motor_speed
    d: int = 0  # direction_angle
    a: int = 0  # servo_angle
    c: int = 0  # command_byte

# 定義 /api/set_control_mode 請求的數據模型
class SetControlModeRequest(BaseModel):
    mode: ControlMode

@router.post("/api/sync", response_model=SyncResponse)
async def sync_data(request: Request, data: SyncRequest):
    global latest_arduino_data, latest_command_sent, latest_thermal_analysis_results
    global current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte
    global current_control_mode

    # 儲存最新收到的數據
    latest_arduino_data = data.model_dump()

    print(f"\n--- Received Sync Data ---")
    print(f"Status Byte (s): {data.s} (Binary: {bin(data.s)})")
    print(f"Voltage (v): {data.v} mV")
    if data.t:
        print(f"Thermal Matrix (t): {data.t}")
        latest_thermal_analysis_results = _analyze_thermal_data(data.t)
        print(f"Thermal Analysis: {latest_thermal_analysis_results}")
    if data.i:
        print(f"ESP32 IP (i): {data.i}")
    print(f"Current Control Mode: {current_control_mode.value}")
    print(f"--------------------------")

    # --- 指令生成邏輯 ---
    response_command: SyncResponse

    if current_control_mode == ControlMode.MANUAL:
        response_command = _generate_manual_commands()
    elif current_control_mode == ControlMode.AVOIDANCE:
        response_command = _generate_avoidance_commands()
    elif current_control_mode == ControlMode.AUTONOMOUS:
        response_command = _generate_autonomous_commands()
    else:
        # 預設情況，例如未知模式，回傳停止指令
        response_command = SyncResponse()

    # 儲存最新發送的指令
    latest_command_sent = response_command.model_dump()

    return response_command

@router.post("/api/manual_control")
async def manual_control(control_data: ManualControlRequest):
    global current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte
    
    current_manual_motor_speed = control_data.m
    current_manual_direction_angle = control_data.d
    current_manual_servo_angle = control_data.a
    current_manual_command_byte = control_data.c

    print(f"\n--- Manual Control Command Received ---")
    print(f"Motor Speed (m): {current_manual_motor_speed}")
    print(f"Direction Angle (d): {current_manual_direction_angle}")
    print(f"Servo Angle (a): {current_manual_servo_angle}")
    print(f"Command Byte (c): {current_manual_command_byte}")
    print(f"-------------------------------------")
    return {"message": "Manual control command updated"}

@router.post("/api/set_control_mode")
async def set_control_mode(request: SetControlModeRequest):
    global current_control_mode
    current_control_mode = request.mode
    print(f"\n--- Control Mode Changed ---")
    print(f"New Control Mode: {current_control_mode.value}")
    print(f"----------------------------")
    return {"message": f"Control mode set to {current_control_mode.value}"}

@router.post("/api/register_camera")
async def register_camera(request_data: RegisterCameraRequest):
    global latest_esp32_cam_ip, camera_processor_instance
    latest_esp32_cam_ip = request_data.i
    print(f"\n--- Received ESP32-S3 IP Registration ---")
    print(f"Registered ESP32-S3 IP: {latest_esp32_cam_ip}")
    print(f"----------------------------------------")
    
    if camera_processor_instance:
        camera_processor_instance.update_stream_source(latest_esp32_cam_ip)
        camera_processor_instance.start()
    else:
        print("Warning: camera_processor_instance is not initialized. Cannot start stream.")

    return {"message": "ESP32-S3 IP registered successfully"}

@router.get("/api/latest_data")
async def get_latest_data():
    return {"latest_data": latest_arduino_data, "latest_command": latest_command_sent, "esp32_cam_ip": latest_esp32_cam_ip, "current_control_mode": current_control_mode.value, "thermal_analysis": latest_thermal_analysis_results, "visual_analysis": camera_processor_instance.get_latest_frame()[2] if camera_processor_instance else None}

# --- 指令生成輔助函式 ---
def _generate_manual_commands() -> SyncResponse:
    return SyncResponse(
        c=current_manual_command_byte,
        m=current_manual_motor_speed,
        d=current_manual_direction_angle,
        a=current_manual_servo_angle
    )

def _generate_avoidance_commands() -> SyncResponse:
    # TODO: 實作避障邏輯
    # 讀取 latest_arduino_data (例如熱成像) 和 CameraStreamProcessor 的分析結果
    # 根據邏輯生成馬達/舵機指令
    print("Executing avoidance logic (placeholder)...")
    return SyncResponse(
        c=current_manual_command_byte,
        m=current_manual_motor_speed,
        d=current_manual_direction_angle,
        a=current_manual_servo_angle
    )

def _generate_autonomous_commands() -> SyncResponse:
    print("Executing autonomous logic...")
    
    # --- Default command: Stop ---
    motor_speed = 0
    direction_angle = 0
    command_byte = 0
    servo_angle = 90 # Center servo

    # --- Get Visual Analysis ---
    visual_analysis = None
    if camera_processor_instance and camera_processor_instance.is_running():
        # The third element [2] contains the analysis results
        visual_analysis = camera_processor_instance.get_latest_frame()[2]

    if not visual_analysis:
        print("  - Visual analysis not available. Stopping.")
        return SyncResponse(c=command_byte, m=motor_speed, d=direction_angle, a=servo_angle)

    print(f"  - Visual Analysis: {visual_analysis}")

    # --- Decision Logic ---
    obstacle_detected = visual_analysis.get("obstacle_detected", False)
    
    if obstacle_detected:
        obstacle_center_x = visual_analysis.get("obstacle_center_x", -1)
        obstacle_area_ratio = visual_analysis.get("obstacle_area_ratio", 0.0)
        
        # Assume frame width is around 320 pixels for this logic
        # This value might need tuning based on the actual camera resolution
        frame_width = 320
        turn_threshold = frame_width / 3 # Divide frame into 3 sections

        print(f"  - Obstacle DETECTED (Center X: {obstacle_center_x}, Area: {obstacle_area_ratio})")

        # Priority 1: Obstacle is very large (close), so back up
        if obstacle_area_ratio > 0.5:
            print("  - Action: Obstacle too close! Moving backward.")
            motor_speed = 100 # Adjust speed as needed
            direction_angle = 180 # Backward
        # Priority 2: Obstacle is on the right, so turn left
        elif obstacle_center_x > (frame_width - turn_threshold):
            print("  - Action: Obstacle on the right. Turning left.")
            motor_speed = 120 # Adjust speed as needed
            direction_angle = 270 # Turn Left
        # Priority 3: Obstacle is on the left, so turn right
        elif obstacle_center_x < turn_threshold:
            print("  - Action: Obstacle on the left. Turning right.")
            motor_speed = 120 # Adjust speed as needed
            direction_angle = 90 # Turn Right
        # Priority 4: Obstacle is in the center, back up slowly
        else:
            print("  - Action: Obstacle in center. Moving backward slowly.")
            motor_speed = 80
            direction_angle = 180 # Backward
    else:
        # --- No Obstacle Detected: Move Forward ---
        print("  - No obstacle detected. Moving forward.")
        motor_speed = 150 # Cruise speed
        direction_angle = 0 # Forward

    return SyncResponse(
        c=command_byte,
        m=motor_speed,
        d=direction_angle,
        a=servo_angle
    )

def _analyze_thermal_data(thermal_matrix: List[List[int]]) -> dict:
    # Convert int*100 to float temperatures
    flat_temps = [temp / 100.0 for row in thermal_matrix for temp in row]
    
    max_temp = max(flat_temps)
    min_temp = min(flat_temps)
    avg_temp = sum(flat_temps) / len(flat_temps)

    # Simple hotspot detection: any temp > 30.0 C
    hotspot_detected = any(temp > 30.0 for temp in flat_temps)

    return {
        "max_temp": round(max_temp, 2),
        "min_temp": round(min_temp, 2),
        "avg_temp": round(avg_temp, 2),
        "hotspot_detected": hotspot_detected
    }
