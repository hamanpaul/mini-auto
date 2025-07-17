API 文件

這份文件描述了 Miniauto 專案後端 FastAPI 服務提供的 API 端點及其功能。

## POST /api/sync
描述
Arduino UNO 向後端同步車輛狀態數據，並接收後端發送的控制指令。

請求範例
```bash
curl -X POST -d '{
  "s": 1,
  "v": 785,
  "t": [[2500, 2510, 2520, 2530, 2540, 2550, 2560, 2570], [2600, 2610, 2620, 2630, 2640, 2650, 2660, 2670], [2700, 2710, 2720, 2730, 2740, 2750, 2760, 2770], [2800, 2810, 2820, 2830, 2840, 2850, 2860, 2870], [2900, 2910, 2920, 2930, 2940, 2950, 2960, 2970], [3000, 3010, 3020, 3030, 3040, 3050, 3060, 3070], [3100, 3110, 3120, 3130, 3140, 3150, 3160, 3170], [3200, 3210, 3220, 3230, 3240, 3250, 3260, 3270]],
  "i": "192.168.1.100"
}' http://your-server-ip:8000/api/sync
```

成功回應
```json
{
  "c": 1,
  "m": 50,
  "d": 90,
  "a": 90
}
```

參數說明
| 參數 | 類型 | 說明 |
|---|---|---|
| **Request** | | |
| `s` | Integer | **狀態字節 (status_byte)**：一個 8 位元整數，用於壓縮表示車輛的各種狀態。每個位元 (bit) 代表一個特定的布林狀態，通過位元運算 (bitwise operations) 進行設置和讀取。例如，如果第 0 位元為 1，表示某個狀態為真；如果第 1 位元為 0，表示另一個狀態為假。具體的位元定義如下：<br><br>| 位元 (Bit) | 狀態描述 | 範例 (位元值) |
|---|---|---|
| 0 | 系統啟動完成 | 1 (已啟動) / 0 (未啟動) |
| 1 | 避障模式啟用 | 1 (啟用) / 0 (禁用) |
| 2 | 循線模式啟用 | 1 (啟用) / 0 (禁用) |
| 3 | 錯誤狀態 (通用) | 1 (有錯誤) / 0 (無錯誤) |
| 4 | 電池低電量警告 | 1 (低電量) / 0 (正常) |
| 5 | 馬達過熱警告 | 1 (過熱) / 0 (正常) |
| 6 | 視覺模組連線狀態 | 1 (已連線) / 0 (未連線) |
| 7 | 熱成像模組連線狀態 | 1 (已連線) / 0 (未連線) | |
| `v` | Integer | 電池電壓 (voltage_mv)，單位毫伏。 |
| `t` | Array of Array of Integer | 熱成像數據 (thermal_matrix)，可選，8x8 矩陣。 |
| `i` | String | ESP32-S3 IP 地址 (esp32_ip)，可選，用於動態註冊。 |
| **Response** | | |
| `c` | Integer | **指令字節 (command_byte)**：一個 8 位元整數，用於壓縮表示後端發送給車輛的各種控制指令。每個位元 (bit) 代表一個特定的布林指令，通過位元運算 (bitwise operations) 進行設置和讀取。車輛會根據這些位元的值來執行相應的動作。具體的位元定義如下：<br><br>| 位元 (Bit) | 指令描述 | 範例 (位元值) |
|---|---|---|
| 0 | 啟動/停止馬達 | 1 (啟動) / 0 (停止) |
| 1 | 啟用避障功能 | 1 (啟用) / 0 (禁用) |
| 2 | 啟用循線功能 | 1 (啟用) / 0 (禁用) |
| 3 | 重置錯誤狀態 | 1 (重置) / 0 (不重置) |
| 4 | 啟用 LED 燈 | 1 (啟用) / 0 (禁用) |
| 5 | 啟用蜂鳴器 | 1 (啟用) / 0 (禁用) |
| 6 | 請求視覺模組數據 | 1 (請求) / 0 (不請求) |
| 7 | 請求熱成像模組數據 | 1 (請求) / 0 (不請求) | |
| `m` | Integer | 馬達速度 (motor_speed)。 |
| `d` | Integer | 方向角度 (direction_angle)。 |
| `a` | Integer | 舵機角度 (servo_angle)。 |

## POST /api/register_camera
描述
Arduino UNO 向後端註冊 ESP32-S3 視覺模組的 IP 地址。

請求範例
```bash
curl -X POST -d '{
  "i": "192.168.1.101"
}' http://your-server-ip:8000/api/register_camera
```

成功回應
```json
{
  "message": "Camera IP registered successfully."
}
```

參數說明
| 參數 | 類型 | 說明 |
|---|---|---|
| **Request** | | |
| `i` | String | ESP32-S3 視覺模組的 IP 地址。 |
| **Response** | | |
| `message` | String | 操作結果訊息。 |

## POST /api/manual_control
描述
設定車輛的手動控制指令 (馬達速度、方向、舵機角度、指令字節)。這些指令將在下一次 /api/sync 回應中發送給 Arduino。

請求範例
```bash
curl -X POST -d '{
  "m": 60,
  "d": 45,
  "a": 120,
  "c": 2
}' http://your-server-ip:8000/api/manual_control
```

成功回應
```json
{
  "message": "Manual control commands set successfully."
}
```

參數說明
| 參數 | 類型 | 說明 |
|---|---|---|
| **Request** | | |
| `m` | Integer | 馬達速度。 |
| `d` | Integer | 方向角度。 |
| `a` | Integer | 舵機角度。 |
| `c` | Integer | 指令字節。 |
| **Response** | | |
| `message` | String | 操作結果訊息。 |

## POST /api/set_control_mode
描述
切換車輛的控制模式 (手動、避障、自主)。

請求範例
```bash
curl -X POST -d '{
  "mode": "avoidance"
}' http://your-server-ip:8000/api/set_control_mode
```

成功回應
```json
{
  "message": "Control mode set to avoidance."
}
```

參數說明
| 參數 | 類型 | 說明 |
|---|---|---|
| **Request** | | |
| `mode` | String | 控制模式，可選值為 "manual", "avoidance", "autonomous"。 |
| **Response** | | |
| `message` | String | 操作結果訊息。 |

## GET /api/latest_data
描述
獲取後端儲存的最新車輛數據、最新發送的指令、ESP32-S3 IP 和當前控制模式。

請求範例
```bash
curl -X GET http://your-server-ip:8000/api/latest_data
```

成功回應
```json
{
  "latest_data": {
    "s": 1,
    "v": 785,
    "t": [[2500, 2510, 2520, 2530, 2540, 2550, 2560, 2570], [2600, 2610, 2620, 2630, 2640, 2650, 2660, 2670], [2700, 2710, 2720, 2730, 2740, 2750, 2760, 2770], [2800, 2810, 2820, 2830, 2840, 2850, 2860, 2870], [2900, 2910, 2920, 2930, 2940, 2950, 2960, 2970], [3000, 3010, 3020, 3030, 3040, 3050, 3060, 3070], [3100, 3110, 3120, 3130, 3140, 3150, 3160, 3170], [3200, 3210, 3220, 3230, 3240, 3250, 3260, 3270]],
    "i": "192.168.1.100"
  },
  "latest_command": {
    "c": 1,
    "m": 50,
    "d": 90,
    "a": 90
  },
  "esp32_cam_ip": "192.168.1.101",
  "current_control_mode": "manual",
  "thermal_analysis": {}, 
  "visual_analysis": {}
}
```

參數說明
| 參數 | 類型 | 說明 |
|---|---|---|
| **Response** | | |
| `latest_data` | Object | 最新收到的 Arduino 數據 (包含 s, v, t, i)。 |
| `latest_command` | Object | 最新發送給 Arduino 的指令 (包含 c, m, d, a)。 |
| `esp32_cam_ip` | String | 註冊的 ESP32-S3 IP 地址。 |
| `current_control_mode` | String | 當前控制模式。 |
| `thermal_analysis` | Object | 熱成像分析結果。 |
| `visual_analysis` | Object | 視覺分析結果。 |