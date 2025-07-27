from fastapi import APIRouter, HTTPException, Body, Depends, Request # 導入 FastAPI 相關模組：APIRouter 用於定義路由，HTTPException 用於處理 HTTP 錯誤，Body 用於從請求體中獲取資料，Depends 用於依賴注入，Request 用於訪問請求物件。
from fastapi import APIRouter, HTTPException, Body, Depends, Request # 導入 FastAPI 相關模組：APIRouter 用於定義路由，HTTPException 用於處理 HTTP 錯誤，Body 用於從請求體中獲取資料，Depends 用於依賴注入，Request 用於訪問請求物件。
from pydantic import BaseModel # 導入 BaseModel，用於定義資料模型，實現資料驗證和序列化。
from enum import Enum # 導入 Enum，用於創建枚舉類型。
from typing import Optional, List # 導入 Optional 和 List，用於型別提示。
from datetime import datetime # 導入 datetime，用於處理日期和時間。
from src.py_rear.apis import camera as apis_camera # 導入 camera 模組，用於存取 camera_processor 實例。

router = APIRouter() # 創建一個 APIRouter 實例，用於定義與車輛控制相關的 API 路由。

# --- 全域日誌緩衝區 ---
backend_log_buffer = [] # 用於儲存後端日誌訊息的列表。
MAX_LOG_ENTRIES = 500 # 限制日誌條目數量，以防止記憶體使用過多。

# 定義一個函數，用於向後端日誌緩衝區添加日誌訊息。
def add_backend_log(message: str, level: str = "INFO"):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S") # 獲取當前時間戳。
    log_entry = f"[{timestamp}] [{level}] [Backend] {message}" # 格式化日誌條目。
    backend_log_buffer.append(log_entry) # 將日誌條目添加到緩衝區。
    if len(backend_log_buffer) > MAX_LOG_ENTRIES: # 如果緩衝區超過最大限制，則移除最舊的條目。
        backend_log_buffer.pop(0) # 移除最舊的條目。

# --- 控制模式定義 ---
class ControlMode(str, Enum): # 定義一個枚舉類，表示車輛的控制模式。
    MANUAL = "manual" # 手動控制模式。
    AVOIDANCE = "avoidance" # 避障模式。
    AUTONOMOUS = "autonomous" # 自動駕駛模式。

current_control_mode: ControlMode = ControlMode.MANUAL # 設定當前的控制模式，預設為手動控制模式。

# 用於儲存當前手動控制命令的全域變數。
current_manual_motor_speed: int = 0 # 馬達速度。
current_manual_direction_angle: int = 0 # 方向角度。
current_manual_servo_angle: int = 0 # 舵機角度。
current_manual_command_byte: int = 0 # 命令位元組。

# 用於儲存最新接收資料和發送命令的全域變數。
latest_arduino_data: Optional[dict] = None # 最新從 Arduino 接收到的資料。
latest_command_sent: Optional[dict] = None # 最新發送給 Arduino 的命令。
latest_esp32_cam_ip: Optional[str] = None # 最新註冊的 ESP32-CAM IP 位址。
latest_thermal_analysis_results: Optional[dict] = None # 最新熱像儀分析結果。
# camera_processor_instance 將從 apis.camera 模組中引用，因為它在 main.py 中被初始化。

# 定義資料模型：SyncRequest，用於同步請求的資料結構。
class SyncRequest(BaseModel):
    s: int  # 狀態位元組 (status_byte)
    v: int  # 電壓 (voltage_mv)
    t: Optional[List[List[int]]] = None  # 熱像儀矩陣 (thermal_matrix)，可選。
    i: Optional[str] = None # ESP32 IP 位址 (esp32_ip)，可選。
    u: Optional[int] = None # 超音波距離 (ultrasonic_distance_cm)，可選。

    # 手動控制欄位 (可選)，用於從 Arduino 接收手動控制指令。
    m: Optional[int] = None  # 馬達速度 (motor_speed)
    d: Optional[int] = None  # 方向角度 (direction_angle)
    a: Optional[int] = None  # 舵機角度 (servo_angle)
    c: Optional[int] = None  # 命令位元組 (command_byte)

# 定義資料模型：SyncResponse，用於同步響應的資料結構。
class SyncResponse(BaseModel):
    c: int = 0  # 命令位元組 (command_byte)
    m: int = 0  # 馬達速度 (motor_speed)
    d: int = 0  # 方向角度 (direction_angle)
    a: int = 0  # 舵機角度 (servo_angle)


# 定義一個 POST 請求的 API 端點：/api/sync，用於同步資料。
@router.post("/api/sync")
async def sync_data(request: SyncRequest) -> SyncResponse:
    global latest_arduino_data, latest_command_sent, latest_thermal_analysis_results, current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte

    latest_arduino_data = request.dict() # 將請求資料轉換為字典並儲存為最新 Arduino 資料。
    print(f"\n--- 接收到同步資料 ---") # 列印接收到的同步資料標題。
    print(f"狀態: {latest_arduino_data.get('s')}") # 列印狀態位元組。
    print(f"電壓: {latest_arduino_data.get('v')}mV") # 列印電壓。
    if latest_arduino_data.get('u') is not None: # 如果有超音波距離資料，則列印。
        print(f"超音波距離: {latest_arduino_data.get('u')}cm")
    if latest_arduino_data.get('t') is not None: # 如果有熱像儀資料，則進行分析並列印結果。
        latest_thermal_analysis_results = _analyze_thermal_data(latest_arduino_data['t'])
        print(f"熱像儀分析: {latest_thermal_analysis_results}")
    print(f"--------------------------") # 列印分隔線。

    # 如果同步請求中提供了手動控制欄位，則更新全域手動控制變數。
    if request.m is not None:
        current_manual_motor_speed = request.m
    if request.d is not None:
        current_manual_direction_angle = request.d
    if request.a is not None:
        current_manual_servo_angle = request.a
    if request.c is not None:
        current_manual_command_byte = request.c

    # 根據當前的控制模式生成命令。
    if current_control_mode == ControlMode.MANUAL:
        response_commands = _generate_manual_commands()
    elif current_control_mode == ControlMode.AVOIDANCE:
        response_commands = _generate_avoidance_commands()
    elif current_control_mode == ControlMode.AUTONOMOUS:
        response_commands = _generate_autonomous_commands()
    else:
        response_commands = SyncResponse() # 預設為停止命令。

    latest_command_sent = response_commands.dict() # 將生成的命令轉換為字典並儲存為最新發送的命令。
    return response_commands # 返回生成的命令。

# 定義 /api/manual_control 請求的數據模型。
class ManualControlRequest(BaseModel):
    m: int  # 馬達速度 (motor_speed)
    d: int  # 方向角度 (direction_angle)
    a: int  # 舵機角度 (servo_angle)
    c: int  # 命令位元組 (command_byte)

# 定義一個 POST 請求的 API 端點：/api/manual_control，用於手動控制車輛。
@router.post("/api/manual_control")
async def manual_control(request: ManualControlRequest):
    global current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte

    current_manual_motor_speed = request.m # 更新馬達速度。
    current_manual_direction_angle = request.d # 更新方向角度。
    current_manual_servo_angle = request.a # 更新舵機角度。
    current_manual_command_byte = request.c # 更新命令位元組。
    print(f"\n--- 接收到手動控制命令 ---") # 列印接收到的手動控制命令標題。
    print(f"馬達速度: {current_manual_motor_speed}") # 列印馬達速度。
    print(f"方向角度: {current_manual_direction_angle}") # 列印方向角度。
    print(f"舵機角度: {current_manual_servo_angle}") # 列印舵機角度。
    print(f"命令位元組: {current_manual_command_byte}") # 列印命令位元組。
    print(f"---------------------------------------") # 列印分隔線。
    return {"message": "手動控制命令已接收"} # 返回確認訊息。

# 定義 /api/register_camera 請求的數據模型。
class RegisterCameraRequest(BaseModel):
    i: str # ESP32 IP 位址 (esp32_ip)

# 定義設定控制模式的請求數據模型。
class SetControlModeRequest(BaseModel):
    mode: ControlMode # 控制模式。

# 定義一個 POST 請求的 API 端點：/api/set_control_mode，用於設定車輛的控制模式。
@router.post("/api/set_control_mode")
async def set_control_mode(request: SetControlModeRequest):
    global current_control_mode # 宣告使用全域變數 current_control_mode。
    current_control_mode = request.mode # 更新當前的控制模式。
    add_backend_log(f"--- 控制模式已變更 ---") # 添加日誌訊息。
    add_backend_log(f"新控制模式: {current_control_mode.value}") # 添加日誌訊息。
    add_backend_log(f"----------------------------") # 添加日誌訊息。
    return {"message": f"控制模式已設定為 {current_control_mode.value}"} # 返回確認訊息。

@router.post("/api/register_camera")
async def register_camera(request_data: RegisterCameraRequest, request: Request):
    global latest_esp32_cam_ip, camera_processor_instance # 宣告使用全域變數。
    latest_esp32_cam_ip = request_data.i # 儲存最新註冊的 ESP32-CAM IP 位址。
    add_backend_log(f"--- 接收到 ESP32-S3 IP 註冊 ---") # 添加日誌訊息。
    add_backend_log(f"已註冊 ESP32-S3 IP: {latest_esp32_cam_ip}") # 添加日誌訊息。
    add_backend_log(f"----------------------------------------") # 添加日誌訊息。
    
    if hasattr(request.app.state, 'broadcast_process') and request.app.state.broadcast_process:
        print("從 API 請求停止 IP 廣播腳本...")
        request.app.state.broadcast_process.terminate()
        request.app.state.broadcast_process = None # 清除進程引用。
        print("廣播腳本已停止。")

    if apis_camera.camera_processor: # 如果 camera_processor 已經初始化。
        apis_camera.camera_processor.update_stream_source(latest_esp32_cam_ip) # 更新影像串流來源。
    else:
        add_backend_log("警告: apis_camera.camera_processor 未初始化。無法啟動串流。", level="WARNING") # 添加警告日誌。

    return {"message": "ESP32-S3 IP 註冊成功"} # 返回確認訊息。

# 定義一個 GET 請求的 API 端點：/api/latest_data，用於獲取最新的資料。
@router.get("/api/latest_data")
async def get_latest_data():
    # 返回最新的 Arduino 資料、最新發送的命令、ESP32-CAM IP、當前控制模式、熱像儀分析結果和視覺分析結果。
    return {"latest_data": latest_arduino_data, "latest_command": latest_command_sent, "esp32_cam_ip": latest_esp32_cam_ip, "current_control_mode": current_control_mode.value, "thermal_analysis": latest_thermal_analysis_results, "visual_analysis": apis_camera.camera_processor.get_latest_frame()[2] if apis_camera.camera_processor else None}

# 定義一個 GET 請求的 API 端點：/api/logs，用於獲取後端日誌。
@router.get("/api/logs")
async def get_logs():
    return {"logs": backend_log_buffer} # 返回後端日誌緩衝區的內容。

# --- 命令生成輔助函數 ---
# 生成手動控制命令。
def _generate_manual_commands() -> SyncResponse:
    return SyncResponse(
        c=current_manual_command_byte, # 命令位元組。
        m=current_manual_motor_speed, # 馬達速度。
        d=current_manual_direction_angle, # 方向角度。
        a=current_manual_servo_angle # 舵機角度。
    )

# 生成避障命令。
def _generate_avoidance_commands() -> SyncResponse:

    # 優先讀取超音波感測器數據。
    if latest_arduino_data and latest_arduino_data.get("u") is not None: # 如果有最新的 Arduino 資料且包含超音波距離。
        distance_cm = latest_arduino_data.get("u") # 獲取超音波距離。
        if distance_cm < 20: # 如果距離小於 20 厘米，表示有障礙物。
            print(f"  - 超音波障礙物偵測到 ({distance_cm} 厘米)。正在後退。") # 列印偵測到障礙物的訊息。
            return SyncResponse(c=0, m=100, d=180, a=90) # 返回後退命令。

    # 如果沒有超音波數據或距離足夠，則執行視覺避障。
    return _generate_autonomous_commands() # 呼叫自動駕駛命令生成函數。

# 生成自動駕駛命令。
def _generate_autonomous_commands() -> SyncResponse:
    print("執行自動駕駛邏輯...") # 列印執行自動駕駛邏輯的訊息。

    # --- 優先級 1: 超音波感測器檢查 ---
    if latest_arduino_data and latest_arduino_data.get("u") is not None: # 如果有最新的 Arduino 資料且包含超音波距離。
        distance_cm = latest_arduino_data.get("u") # 獲取超音波距離。
        if distance_cm < 20: # 如果距離小於 20 厘米，表示有障礙物。
            print(f"  - 超音波障礙物偵測到 ({distance_cm} 厘米)。正在後退。") # 列印偵測到障礙物的訊息。
            return SyncResponse(c=0, m=100, d=180, a=90) # 返回後退命令。
    
    # --- 預設命令: 停止 ---
    motor_speed = 0 # 馬達速度設為 0 (停止)。
    direction_angle = 0 # 方向角度設為 0。
    command_byte = 0 # 命令位元組設為 0。
    servo_angle = 90 # 舵機角度設為 90 (置中)。

    # --- 獲取視覺分析結果 ---
    visual_analysis = None # 初始化視覺分析結果為 None。
    if apis_camera.camera_processor and apis_camera.camera_processor.is_running(): # 如果 camera_processor 存在且正在運行。
        # 第三個元素 [2] 包含分析結果。
        visual_analysis = apis_camera.camera_processor.get_latest_frame()[2]

    if not visual_analysis: # 如果沒有視覺分析結果。
        add_backend_log("  - 視覺分析不可用。正在停止。", level="DEBUG") # 添加日誌訊息。
        return SyncResponse(c=command_byte, m=motor_speed, d=direction_angle, a=servo_angle) # 返回停止命令。

    add_backend_log(f"  - 視覺分析: {visual_analysis}", level="DEBUG") # 添加日誌訊息。

    # --- 決策邏輯 ---
    obstacle_detected = visual_analysis.get("obstacle_detected", False) # 獲取是否偵測到障礙物。
    
    if obstacle_detected: # 如果偵測到障礙物。
        obstacle_center_x = visual_analysis.get("obstacle_center_x", -1) # 獲取障礙物中心 X 座標。
        obstacle_area_ratio = visual_analysis.get("obstacle_area_ratio", 0.0) # 獲取障礙物面積比例。
        
        # 由於相機解析度已調整為 QQVGA (160x120)，因此將幀寬度調整為 160。
        # 這個值可能需要根據實際相機解析度進行微調。
        frame_width = 480 # 幀寬度。
        turn_threshold = frame_width / 3 # 轉向閾值，將幀分為 3 個區域。

        add_backend_log(f"  - 障礙物偵測到 (中心 X: {obstacle_center_x}, 面積: {obstacle_area_ratio})", level="DEBUG") # 添加日誌訊息。

        # 優先級 1: 障礙物非常大（靠近），因此後退。
        if obstacle_area_ratio > 0.5:
            add_backend_log("  - 動作: 障礙物太近！正在後退。", level="DEBUG") # 添加日誌訊息。
            motor_speed = 100 # 調整速度。
            direction_angle = 180 # 後退。
        # 優先級 2: 障礙物在右側，因此向左轉。
        elif obstacle_center_x > (frame_width - turn_threshold):
            add_backend_log("  - 動作: 障礙物在右側。正在向左轉。", level="DEBUG") # 添加日誌訊息。
            motor_speed = 120 # 調整速度。
            direction_angle = 270 # 向左轉。
        # 優先級 3: 障礙物在左側，因此向右轉。
        elif obstacle_center_x < turn_threshold:
            add_backend_log("  - 動作: 障礙物在左側。正在向右轉。", level="DEBUG") # 添加日誌訊息。
            motor_speed = 120 # 調整速度。
            direction_angle = 90 # 向右轉。
        # 優先級 4: 障礙物在中間，緩慢後退。
        else:
            add_backend_log("  - 動作: 障礙物在中間。正在緩慢後退。", level="DEBUG") # 添加日誌訊息。
            motor_speed = 80
            direction_angle = 180 # 後退。
    else:
        # --- 未偵測到障礙物: 前進 ---
        add_backend_log("  - 未偵測到障礙物。正在前進。", level="DEBUG") # 添加日誌訊息。
        motor_speed = 150 # 巡航速度。
        direction_angle = 0 # 前進。

    return SyncResponse(
        c=command_byte, # 命令位元組。
        m=motor_speed, # 馬達速度。
        d=direction_angle, # 方向角度。
        a=servo_angle # 舵機角度。
    )

# 分析熱像儀資料。
def _analyze_thermal_data(thermal_matrix: List[List[int]]) -> dict:
    # 將整數值（乘以 100）轉換為浮點數溫度。
    flat_temps = [temp / 100.0 for row in thermal_matrix for temp in row]
    
    max_temp = max(flat_temps) # 計算最高溫度。
    min_temp = min(flat_temps) # 計算最低溫度。
    avg_temp = sum(flat_temps) / len(flat_temps) # 計算平均溫度。

    # 簡單的熱點偵測：任何溫度高於 30.0 攝氏度。
    hotspot_detected = any(temp > 30.0 for temp in flat_temps) # 檢查是否存在熱點。

    return {
        "max_temp": round(max_temp, 2), # 返回最高溫度，保留兩位小數。
        "min_temp": round(min_temp, 2), # 返回最低溫度，保留兩位小數。
        "avg_temp": round(avg_temp, 2), # 返回平均溫度，保留兩位小數。
        "hotspot_detected": hotspot_detected # 返回是否偵測到熱點。
    }