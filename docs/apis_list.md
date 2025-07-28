# API 文件

這份文件描述了 Miniauto 專案後端 FastAPI 服務提供的 API 端點及其功能。

## POST /api/sync
**描述**
ESP32 網路代理向後端同步從 UNO 接收的車輛狀態數據，並接收後端發送的控制指令。

**請求範例**
```bash
curl -X POST -d '{
  "s": 1,
  "v": 785,
  "t": [[2500, 2510, ...]],
  "u": 50
}' http://your-server-ip:8000/api/sync
```

**成功回應**
```json
{
  "c": 1,
  "m": 50,
  "d": 90,
  "a": 90
}
```

**參數說明**
| 參數 | 類型 | 說明 |
|---|---|---|
| **Request** | | |
| `s` | Integer | **狀態字節 (status_byte)**：一個 8 位元整數，表示車輛的各種狀態。 |
| `v` | Integer | 電池電壓 (voltage_mv)，單位毫伏。 |
| `t` | Array of Array of Integer | 熱成像數據 (thermal_matrix)，可選，8x8 矩陣。 |
| `u` | Integer | 超音波距離 (ultrasonic_distance_cm)，可選。 |
| **Response** | | |
| `c` | Integer | **指令字節 (command_byte)**：一個 8 位元整數，表示後端發送的控制指令。 |
| `m` | Integer | 馬達速度 (motor_speed)。 |
| `d` | Integer | 方向角度 (direction_angle)。 |
| `a` | Integer | 舵機角度 (servo_angle)。 |

## POST /api/register_camera
**描述**
ESP32-CAM 向後端註冊自身的 IP 地址。

**請求範例**
```bash
curl -X POST -d '{
  "i": "192.168.1.101"
}' http://your-server-ip:8000/api/register_camera
```

**成功回應**
```json
{
  "message": "Camera IP registered successfully."
}
```

## POST /api/manual_control
**描述**
從 GUI 設定車輛的手動控制指令。這些指令將在下一次 `/api/sync` 回應中發送給 ESP32。

**請求範例**
```bash
curl -X POST -d '{
  "m": 60,
  "d": 45,
  "a": 120,
  "c": 2
}' http://your-server-ip:8000/api/manual_control
```

## POST /api/set_control_mode
**描述**
從 GUI 切換車輛的控制模式 (手動、避障、自主)。

**請求範例**
```bash
curl -X POST -d '{
  "mode": "avoidance"
}' http://your-server-ip:8000/api/set_control_mode
```

## GET /api/latest_data
**描述**
從 GUI 獲取後端儲存的最新車輛數據、指令、IP 和模式。

**請求範例**
```bash
curl -X GET http://your-server-ip:8000/api/latest_data
```

**成功回應**
```json
{
  "latest_data": {
    "s": 1, "v": 785, "t": [...], "u": 50
  },
  "latest_command": {
    "c": 1, "m": 50, "d": 90, "a": 90
  },
  "esp32_cam_ip": "192.168.1.101",
  "current_control_mode": "manual",
  "thermal_analysis": {}, 
  "visual_analysis": {}
}
```