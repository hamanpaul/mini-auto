from fastapi import APIRouter, HTTPException, Body, Depends, Request
from pydantic import BaseModel
from enum import Enum
from typing import Optional, List
from datetime import datetime

router = APIRouter()

# --- Global Log Buffer ---
backend_log_buffer = []
MAX_LOG_ENTRIES = 500 # Limit log entries to prevent excessive memory usage

def add_backend_log(message: str, level: str = "INFO"):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_entry = f"[{timestamp}] [{level}] [Backend] {message}"
    backend_log_buffer.append(log_entry)
    if len(backend_log_buffer) > MAX_LOG_ENTRIES:
        backend_log_buffer.pop(0) # Remove oldest entry if buffer exceeds limit

# --- Control Mode Definition ---
class ControlMode(str, Enum):
    MANUAL = "manual"
    AVOIDANCE = "avoidance"
    AUTONOMOUS = "autonomous"

current_control_mode: ControlMode = ControlMode.MANUAL # Default to manual control mode

# Global variables for current manual control commands
current_manual_motor_speed: int = 0
current_manual_direction_angle: int = 0
current_manual_servo_angle: int = 0
current_manual_command_byte: int = 0

# Global variables for latest received data and sent commands
latest_arduino_data: Optional[dict] = None
latest_command_sent: Optional[dict] = None
latest_esp32_cam_ip: Optional[str] = None
latest_thermal_analysis_results: Optional[dict] = None
camera_processor_instance = None # CameraStreamProcessor instance

# Define data models
class SyncRequest(BaseModel):
    s: int  # status_byte
    v: int  # voltage_mv
    t: Optional[List[List[int]]] = None  # thermal_matrix (optional)
    i: Optional[str] = None # esp32_ip (optional)
    u: Optional[int] = None # ultrasonic_distance_cm (optional)

class SyncResponse(BaseModel):
    c: int = 0  # command_byte
    m: int = 0  # motor_speed
    d: int = 0  # direction_angle
    a: int = 0  # servo_angle

class RegisterCameraRequest(BaseModel):
    i: str # esp32_ip

class ManualControlRequest(BaseModel):
    m: int = 0  # motor_speed
    d: int = 0  # direction_angle
    a: int = 0  # servo_angle
    c: int = 0  # command_byte

class SetControlModeRequest(BaseModel):
    mode: ControlMode

@router.post("/api/sync", response_model=SyncResponse)
async def sync_data(request: Request, data: SyncRequest):
    global latest_arduino_data, latest_command_sent, latest_thermal_analysis_results
    global current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte
    global current_control_mode

    # Store latest received data
    latest_arduino_data = data.model_dump()

    add_backend_log(f"--- Received Sync Data ---")
    add_backend_log(f"Status Byte (s): {data.s} (Binary: {bin(data.s)})")
    add_backend_log(f"Voltage (v): {data.v} mV")
    if data.t:
        add_backend_log(f"Thermal Matrix (t): {data.t}")
        latest_thermal_analysis_results = _analyze_thermal_data(data.t)
        add_backend_log(f"Thermal Analysis: {latest_thermal_analysis_results}")
    if data.i:
        add_backend_log(f"ESP32 IP (i): {data.i}")
    if data.u is not None:
        add_backend_log(f"Ultrasonic Distance (u): {data.u} cm")
    add_backend_log(f"Current Control Mode: {current_control_mode.value}")
    add_backend_log(f"--------------------------")

    # --- Command Generation Logic ---
    response_command: SyncResponse

    if current_control_mode == ControlMode.MANUAL:
        response_command = _generate_manual_commands()
    elif current_control_mode == ControlMode.AVOIDANCE:
        response_command = _generate_avoidance_commands()
    elif current_control_mode == ControlMode.AUTONOMOUS:
        response_command = _generate_autonomous_commands()
    else:
        # Default case, e.g., unknown mode, return stop command
        response_command = SyncResponse()

    # Store latest sent command
    latest_command_sent = response_command.model_dump()

    return response_command

@router.post("/api/manual_control")
async def manual_control(control_data: ManualControlRequest):
    global current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte
    
    current_manual_motor_speed = control_data.m
    current_manual_direction_angle = control_data.d
    current_manual_servo_angle = control_data.a
    current_manual_command_byte = control_data.c

    add_backend_log(f"--- Manual Control Command Received ---")
    add_backend_log(f"Motor Speed (m): {current_manual_motor_speed}")
    add_backend_log(f"Direction Angle (d): {current_manual_direction_angle}")
    add_backend_log(f"Servo Angle (a): {current_manual_servo_angle}")
    add_backend_log(f"Command Byte (c): {current_manual_command_byte}")
    add_backend_log(f"-------------------------------------")
    return {"message": "Manual control command updated"}

@router.post("/api/set_control_mode")
async def set_control_mode(request: SetControlModeRequest):
    global current_control_mode
    current_control_mode = request.mode
    add_backend_log(f"--- Control Mode Changed ---")
    add_backend_log(f"New Control Mode: {current_control_mode.value}")
    add_backend_log(f"----------------------------")
    return {"message": f"Control mode set to {current_control_mode.value}"}

@router.post("/api/register_camera")
async def register_camera(request_data: RegisterCameraRequest):
    global latest_esp32_cam_ip, camera_processor_instance
    latest_esp32_cam_ip = request_data.i
    add_backend_log(f"--- Received ESP32-S3 IP Registration ---")
    add_backend_log(f"Registered ESP32-S3 IP: {latest_esp32_cam_ip}")
    add_backend_log(f"----------------------------------------")
    
    if camera_processor_instance:
        camera_processor_instance.update_stream_source(latest_esp32_cam_ip)
        camera_processor_instance.start()
    else:
        add_backend_log("Warning: camera_processor_instance is not initialized. Cannot start stream.", level="WARNING")

    return {"message": "ESP32-S3 IP registered successfully"}

@router.get("/api/latest_data")
async def get_latest_data():
    return {"latest_data": latest_arduino_data, "latest_command": latest_command_sent, "esp32_cam_ip": latest_esp32_cam_ip, "current_control_mode": current_control_mode.value, "thermal_analysis": latest_thermal_analysis_results, "visual_analysis": camera_processor_instance.get_latest_frame()[2] if camera_processor_instance else None}

@router.get("/api/logs")
async def get_logs():
    return {"logs": backend_log_buffer}

# --- Command Generation Helper Functions ---
def _generate_manual_commands() -> SyncResponse:
    return SyncResponse(
        c=current_manual_command_byte,
        m=current_manual_motor_speed,
        d=current_manual_direction_angle,
        a=current_manual_servo_angle
    )

def _generate_avoidance_commands() -> SyncResponse:
    # TODO: Implement avoidance logic
    # Read latest_arduino_data (e.g., thermal imaging) and analysis results from CameraStreamProcessor
    # Generate motor/servo commands based on logic
    add_backend_log("Executing avoidance logic (placeholder)...", level="DEBUG")
    return SyncResponse(
        c=current_manual_command_byte,
        m=current_manual_motor_speed,
        d=current_manual_direction_angle,
        a=current_manual_servo_angle
    )

def _generate_autonomous_commands() -> SyncResponse:
    add_backend_log("Executing autonomous logic...", level="DEBUG")
    
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
        add_backend_log("  - Visual analysis not available. Stopping.", level="DEBUG")
        return SyncResponse(c=command_byte, m=motor_speed, d=direction_angle, a=servo_angle)

    add_backend_log(f"  - Visual Analysis: {visual_analysis}", level="DEBUG")

    # --- Decision Logic ---
    obstacle_detected = visual_analysis.get("obstacle_detected", False)
    
    if obstacle_detected:
        obstacle_center_x = visual_analysis.get("obstacle_center_x", -1)
        obstacle_area_ratio = visual_analysis.get("obstacle_area_ratio", 0.0)
        
        # Assume frame width is around 320 pixels for this logic
        # This value might need tuning based on the actual camera resolution
        frame_width = 320
        turn_threshold = frame_width / 3 # Divide frame into 3 sections

        add_backend_log(f"  - Obstacle DETECTED (Center X: {obstacle_center_x}, Area: {obstacle_area_ratio})", level="DEBUG")

        # Priority 1: Obstacle is very large (close), so back up
        if obstacle_area_ratio > 0.5:
            add_backend_log("  - Action: Obstacle too close! Moving backward.", level="DEBUG")
            motor_speed = 100 # Adjust speed as needed
            direction_angle = 180 # Backward
        # Priority 2: Obstacle is on the right, so turn left
        elif obstacle_center_x > (frame_width - turn_threshold):
            add_backend_log("  - Action: Obstacle on the right. Turning left.", level="DEBUG")
            motor_speed = 120 # Adjust speed as needed
            direction_angle = 270 # Turn Left
        # Priority 3: Obstacle is on the left, so turn right
        elif obstacle_center_x < turn_threshold:
            add_backend_log("  - Action: Obstacle on the left. Turning right.", level="DEBUG")
            motor_speed = 120 # Adjust speed as needed
            direction_angle = 90 # Turn Right
        # Priority 4: Obstacle is in the center, back up slowly
        else:
            add_backend_log("  - Action: Obstacle in center. Moving backward slowly.", level="DEBUG")
            motor_speed = 80
            direction_angle = 180 # Backward
    else:
        # --- No Obstacle Detected: Move Forward ---
        add_backend_log("  - No obstacle detected. Moving forward.", level="DEBUG")
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