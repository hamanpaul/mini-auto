# API 與功能測試對應表

這份文件列出了後端 FastAPI 服務的 API 端點與其對應的功能測試腳本。

## API 端點與測試腳本對應

| API 端點 | 描述 | 對應的功能測試腳本 |
|---|---|---|
| `POST /api/sync` | ESP32 向後端同步車輛狀態數據，並接收後端發送的控制指令。 | `test/feature/test_sync_thermal.sh`<br>`test/feature/test_sync_no_thermal.sh` |
| `POST /api/register_camera` | ESP32 向後端註冊自身的 IP 地址。 | `test/feature/test_register_camera_api.sh`<br>`test/feature/test_register_camera.sh` |
| `POST /api/manual_control` | 設定車輛的手動控制指令 (馬達速度、方向、舵機角度、指令字節)。 | `test/feature/test_manual_control.sh` |
| `POST /api/set_control_mode` | 切換車輛的控制模式 (手動、避障、自主)。 | `test/feature/test_set_control_mode_manual.sh`<br>`test/feature/test_set_control_mode_avoidance.sh` |
| `GET /api/latest_data` | 獲取後端儲存的最新車輛數據、最新發送的指令、ESP32-S3 IP 和當前控制模式。 | `test/feature/test_get_latest_data.sh` |
