from fastapi import APIRouter, HTTPException, Body, Depends, Request # 導入 FastAPI 相關模組：APIRouter 用於定義路由，HTTPException 用於處理 HTTP 錯誤，Body 用於從請求體中獲取資料，Depends 用於依賴注入，Request 用於訪問請求物件。
import time # 導入 time 模組，用於處理時間戳
from fastapi import APIRouter, HTTPException, Body, Depends, Request # 導入 FastAPI 相關模組：APIRouter 用於定義路由，HTTPException 用於處理 HTTP 錯誤，Body 用於從請求體中獲取資料，Depends 用於依賴注入，Request 用於訪問請求物件。
from pydantic import BaseModel # 導入 BaseModel，用於定義資料模型，實現資料驗證和序列化。
from enum import Enum # 導入 Enum，用於創建枚舉類型。
from typing import Optional, List, Any # 導入 Optional 和 List，用於型別提示。
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
current_manual_rotation_speed: int = 0 # 新增：旋轉速度。

# 用於儲存最新接收資料和發送命令的全域變數。
latest_arduino_data: Optional[dict] = None # 最新從 Arduino 接收到的資料。
latest_command_sent: Optional[dict] = None # 最新發送給 Arduino 的命令。
latest_esp32_cam_ip: Optional[str] = None # 最新註冊的 ESP32-CAM IP 位址。
latest_thermal_analysis_results: Optional[dict] = None # 最新熱像儀分析結果。

# 用於儲存被忽略的障礙物角度和時間戳
# 每個元素: {'angle': float, 'ignore_until': float (timestamp)}
global_obstacle_map = []

# 輔助函式：將像素座標轉換為角度
def _pixel_to_angle(pixel_coord: int, frame_dim: int, fov_degrees: float) -> float:
    # 將像素座標正規化到 -1.0 到 1.0 的範圍 (中心為 0)
    normalized_coord = (pixel_coord - (frame_dim / 2.0)) / (frame_dim / 2.0)
    # 將正規化後的座標乘以 fov_degrees / 2，得到角度
    return normalized_coord * (fov_degrees / 2.0)

# camera_processor_instance 將從 apis.camera 模組中引用，因為它在 main.py 中被初始化。

# 定義資料模型：SyncRequest，用於同步請求的資料結構。
class SyncRequest(BaseModel):
    s: int  # status_byte
    v: int  # voltage_mv
    u: Optional[int] = None  # ultrasonic_distance_cm
    # 熱成像特徵值
    t_max: Optional[int] = None  # thermal_max_temp
    t_min: Optional[int] = None  # thermal_min_temp
    t_hx: Optional[int] = None  # thermal_hotspot_x
    t_hy: Optional[int] = None  # thermal_hotspot_y
    i: Optional[str] = None  # esp32_ip

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
    r: int = 0  # 新增：旋轉速度 (rotation_speed)
    is_avoidance_enabled: int = 0 # 避障啟用旗標


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
    if latest_arduino_data.get('t_max') is not None: # 如果有熱像儀資料，則進行分析並列印結果。
        latest_thermal_analysis_results = _analyze_thermal_data(latest_arduino_data['t_max'], latest_arduino_data['t_min'], latest_arduino_data['t_hx'], latest_arduino_data['t_hy'])
        print(f"熱像儀分析: {latest_thermal_analysis_results}")
    print(f"--------------------------") # 列印分隔線。


    if request.m is not None:
        current_manual_motor_speed = request.m
    if request.d is not None:
        current_manual_direction_angle = request.d
    if request.a is not None:
        current_manual_servo_angle = request.a
    if request.c is not None:
        current_manual_command_byte = request.c

    # 根據當前的控制模式生成命令。
    if current_control_mode == ControlMode.AUTONOMOUS:
        response_commands = _generate_autonomous_commands()
    else:
        is_avoidance_enabled = 1 if current_control_mode == ControlMode.AVOIDANCE else 0
        response_commands = SyncResponse(
            c=current_manual_command_byte,
            m=current_manual_motor_speed,
            d=current_manual_direction_angle,
            a=current_manual_servo_angle,
            r=current_manual_rotation_speed,
            is_avoidance_enabled=is_avoidance_enabled
        )

    # 如果熱像儀偵測到危險，強制設定 command_byte 觸發蜂鳴器
    if latest_thermal_analysis_results and latest_thermal_analysis_results.get("is_danger"):
        response_commands.c = 3 # 3 代表連續蜂鳴

    latest_command_sent = response_commands.dict() # 將生成的命令轉換為字典並儲存為最新發送的命令。
    return response_commands # 返回生成的命令。


# 定義 /api/manual_control 請求的數據模型。
class ManualControlRequest(BaseModel):
    m: int  # 馬達速度 (motor_speed)
    d: int  # 方向角度 (direction_angle)
    a: int  # 舵機角度 (servo_angle)
    c: int  # 命令位元組 (command_byte)
    r: int  # 新增：旋轉速度 (rotation_speed)

# 定義一個 POST 請求的 API 端點：/api/manual_control，用於手動控制車輛。
@router.post("/api/manual_control")
async def manual_control(request: ManualControlRequest):
    global current_manual_motor_speed, current_manual_direction_angle, current_manual_servo_angle, current_manual_command_byte, current_manual_rotation_speed

    current_manual_motor_speed = request.m # 更新馬達速度。
    current_manual_direction_angle = request.d # 更新方向角度。
    current_manual_servo_angle = request.a # 更新舵機角度。
    current_manual_command_byte = request.c # 更新命令位元組。
    current_manual_rotation_speed = request.r # 新增：更新旋轉速度。
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

# 生成自動駕駛命令。
def _generate_autonomous_commands() -> SyncResponse:
    global global_obstacle_map

    motor_speed = 0
    direction_angle = 0
    command_byte = 0
    servo_angle = 90 # 置中

    current_time = time.time()

    # 1. 清理 global_obstacle_map：移除所有過期的障礙物
    global_obstacle_map[:] = [
        item for item in global_obstacle_map if item['ignore_until'] > current_time
    ]
    add_backend_log(f"清理後障礙物地圖: {global_obstacle_map}", level="DEBUG")

    # 2. 獲取感測器數據並轉換為角度
    thermal_angle = None
    if latest_thermal_analysis_results and latest_thermal_analysis_results.get("hotspot_detected"):
        thermal_hotspot_x = latest_thermal_analysis_results.get("hotspot_x")
        # 熱像儀是 8x8 像素，視角 60 度
        thermal_angle = _pixel_to_angle(thermal_hotspot_x, 8, 60)
        add_backend_log(f"熱源角度: {thermal_angle:.2f} 度", level="DEBUG")

    visual_obstacle_angle = None
    obstacle_detected_by_vision = False
    obstacle_area_ratio = 0.0

    visual_analysis = None
    if apis_camera.camera_processor and apis_camera.camera_processor.is_running():
        visual_analysis = apis_camera.camera_processor.get_latest_frame()[2]

    if visual_analysis:
        obstacle_detected_by_vision = visual_analysis.get("obstacle_detected", False)
        if obstacle_detected_by_vision:
            obstacle_center_x = visual_analysis.get("obstacle_center_x", -1)
            obstacle_area_ratio = visual_analysis.get("obstacle_area_ratio", 0.0)
            # 攝影機解析度 480x320 (假設)，視角 62 度
            visual_obstacle_angle = _pixel_to_angle(obstacle_center_x, 480, 62)
            add_backend_log(f"視覺障礙物角度: {visual_obstacle_angle:.2f} 度, 面積: {obstacle_area_ratio:.2f}", level="DEBUG")

    # 3. 決策優先級：即時避障 (最高優先)
    if obstacle_detected_by_vision:
        add_backend_log("偵測到視覺障礙物，執行避障。", level="INFO")
        # 將障礙物加入 global_obstacle_map
        # 忽略時間設定為 10 秒，給車輛足夠時間繞開
        ignore_until_timestamp = current_time + 10 
        global_obstacle_map.append({
            'angle': visual_obstacle_angle,
            'ignore_until': ignore_until_timestamp
        })
        add_backend_log(f"障礙物 {visual_obstacle_angle:.2f} 度已加入忽略列表，直到 {ignore_until_timestamp:.0f}", level="DEBUG")

        if obstacle_area_ratio > 0.5: # 障礙物非常大（靠近），因此後退
            add_backend_log("動作: 障礙物太近！正在後退。", level="INFO")
            motor_speed = 100
            direction_angle = 180 # 後退
        else: # 嘗試繞行
            # 判斷障礙物在左還是右，然後往反方向繞行
            if visual_obstacle_angle > 0: # 障礙物在右側，向左繞行
                add_backend_log("動作: 障礙物在右側。正在向左繞行。", level="INFO")
                motor_speed = 80 # 繞行速度
                direction_angle = 270 # 向左轉
            else: # 障礙物在左側，向右繞行
                add_backend_log("動作: 障礙物在左側。正在向右繞行。", level="INFO")
                motor_speed = 80 # 繞行速度
                direction_angle = 90 # 向右轉
        
        return SyncResponse(c=command_byte, m=motor_speed, d=direction_angle, a=servo_angle, is_avoidance_enabled=0)

    # 4. 決策優先級：熱源追蹤
    if thermal_angle is not None:
        # 檢查熱源是否在 global_obstacle_map 中被忽略
        is_thermal_ignored = False
        for item in global_obstacle_map:
            # 如果熱源角度與被忽略的障礙物角度非常接近 (例如，角度差小於 15 度)
            if abs(thermal_angle - item['angle']) < 15: 
                is_thermal_ignored = True
                add_backend_log(f"熱源 {thermal_angle:.2f} 度被忽略 (與障礙物 {item['angle']:.2f} 度衝突)。", level="INFO")
                break
        
        if not is_thermal_ignored:
            add_backend_log(f"動作: 追蹤熱源 {thermal_angle:.2f} 度。", level="INFO")
            motor_speed = 45 # 巡航速度
            # 根據熱源角度調整方向
            if thermal_angle > 5: # 熱源在右側，向右轉
                direction_angle = 90 
            elif thermal_angle < -5: # 熱源在左側，向左轉
                direction_angle = 270
            else: # 熱源在正前方，前進
                direction_angle = 0
            
            return SyncResponse(c=command_byte, m=motor_speed, d=direction_angle, a=servo_angle, is_avoidance_enabled=0)

    # 5. 決策優先級：自由巡航 (無目標或目標被擋)
    add_backend_log("動作: 無熱源或熱源被忽略，執行自由巡航。", level="INFO")
    motor_speed = 30 # 巡航速度
    direction_angle = 270 # 緩慢原地旋轉 (向左) 尋找目標

    return SyncResponse(c=command_byte, m=motor_speed, d=direction_angle, a=servo_angle, is_avoidance_enabled=0)




# 分析熱像儀資料。
def _analyze_thermal_data(thermal_max_temp: Optional[int], thermal_min_temp: Optional[int], thermal_hotspot_x: Optional[int], thermal_hotspot_y: Optional[int]) -> dict[str, Any]:
    if thermal_max_temp is None or thermal_min_temp is None:
        return {"status": "no_data"}

    # 將溫度值從 *100 的格式轉換回來
    max_temp_c = thermal_max_temp / 100.0
    min_temp_c = thermal_min_temp / 100.0

    # 這裡可以根據需求添加更複雜的分析邏輯
    # 例如，判斷是否有熱點、溫度閾值等
    hotspot_detected = False
    if thermal_max_temp > 3500: # 假設超過 35 攝氏度為熱點
        hotspot_detected = True

    return {
        "max_temp": max_temp_c,
        "min_temp": min_temp_c,
        "hotspot_x": thermal_hotspot_x,
        "hotspot_y": thermal_hotspot_y,
        "hotspot_detected": hotspot_detected,
        "is_danger": hotspot_detected # 當 hotspot_detected 為 True 時，表示危險
    }
