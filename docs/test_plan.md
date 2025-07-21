# 後端功能性驗證測試計畫 (Backend Functional Validation Test Plan)

本測試計畫旨在驗證 Miniauto 專案後端 FastAPI 服務的各項功能是否符合預期。

## 測試環境設定
- 確保 FastAPI 服務已在 `http://127.0.0.1:8000` 運行。
- 使用 `curl` 命令進行 API 呼叫。

## 測試項目、流程與預期結果

### 1. POST /api/sync

#### 測試項目
- 驗證車輛狀態數據的同步功能。
- 驗證後端根據控制模式返回正確的指令。

#### 測試流程
1. **同步帶有熱成像數據的狀態**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/sync \
          -H "Content-Type: application/json" \
          -d '{"s": 79, "v": 785, "t": [[2550, 2600], [2575, 2625]], "i": "192.168.1.200"}'
     ```
   - **預期結果**: 返回 JSON 格式的指令，例如 `{"c":0,"m":0,"d":0,"a":0}` (具體值取決於當前控制模式的預設行為)。

2. **同步不帶熱成像數據的狀態**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/sync \
          -H "Content-Type: application/json" \
          -d '{"s": 78, "v": 750}'
     ```
   - **預期結果**: 返回 JSON 格式的指令，例如 `{"c":0,"m":0,"d":0,"a":0}`。

### 2. POST /api/register_camera

#### 測試項目
- 驗證 ESP32-S3 視覺模組 IP 地址的註冊功能。

#### 測試流程
1. **註冊攝影機 IP**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/register_camera \
          -H "Content-Type: application/json" \
          -d '{"i": "192.168.1.100"}'
     ```
   - **預期結果**: 返回 `{"message":"ESP32-S3 IP registered successfully"}`。

### 3. POST /api/manual_control

#### 測試項目
- 驗證手動控制指令的設定功能。

#### 測試流程
1. **設定手動控制指令**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/manual_control \
          -H "Content-Type: application/json" \
          -d '{"m": 50, "d": 90, "a": 45, "c": 1}'
     ```
   - **預期結果**: 返回 `{"message":"Manual control command updated"}`。

### 4. POST /api/set_control_mode

#### 測試項目
- 驗證車輛控制模式的切換功能。

#### 測試流程
1. **切換至避障模式**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/set_control_mode \
          -H "Content-Type: application/json" \
          -d '{"mode": "avoidance"}'
     ```
   - **預期結果**: 返回 `{"message":"Control mode set to avoidance"}`。

2. **切換至手動模式**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/set_control_mode \
          -H "Content-Type: application/json" \
          -d '{"mode": "manual"}'
     ```
   - **預期結果**: 返回 `{"message":"Control mode set to manual"}`。

### 5. GET /api/latest_data

#### 測試項目
- 驗證後端儲存的最新數據和指令的獲取功能。

#### 測試流程
1. **準備數據 (設定手動控制和模式，並同步一次)**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/manual_control \
          -H "Content-Type: application/json" \
          -d '{"m": 60, "d": 180, "a": 70, "c": 2}'
     curl -X POST http://127.0.0.1:8000/api/set_control_mode \
          -H "Content-Type: application/json" \
          -d '{"mode": "autonomous"}'
     curl -X POST http://127.0.0.1:8000/api/register_camera \
          -H "Content-Type: application/json" \
          -d '{"i": "192.168.1.200"}'
     curl -X POST http://127.0.0.1:8000/api/sync \
          -H "Content-Type: application/json" \
          -d '{"s": 100, "v": 800, "t": [[100, 200], [300, 400]], "i": "192.168.1.200"}'
     ```
   - **預期結果**: 各個 API 呼叫返回成功訊息。

2. **獲取最新數據**
   - **命令**:
     ```bash
     curl -X GET http://127.0.0.1:8000/api/latest_data
     ```
   - **預期結果**: 返回包含最新數據的 JSON 物件，例如：
     ```json
     {
       "latest_data": {"s": 100, "v": 800, "t": [[100, 200], [300, 400]], "i": "192.168.1.200"},
       "latest_command": {"c": 2, "m": 60, "d": 180, "a": 70},
       "esp32_cam_ip": "192.168.1.200",
       "current_control_mode": "autonomous",
       "thermal_analysis": {"max_temp": 4.0, "min_temp": 1.0, "avg_temp": 2.5, "hotspot_detected": false},
       "visual_analysis": null
     }
     ```

## 驗證方法
- 觀察 `curl` 命令的輸出，與「預期結果」進行比對。
- 檢查 FastAPI 服務的控制台輸出 (如果運行在前景或查看 `server.log`)，確認是否有錯誤或異常訊息。

## Useage
- 下載專案檔https://github.com/hamanpaul/mini-auto/archive/refs/heads/main.zip
- 解壓縮，請注意路不要有中文
- 以管理員身份啟動powershell
- 設定RemoteSigned

```
PS C:\Windows\system32> Set-ExecutionPolicy RemoteSigned

執行原則變更
執行原則有助於防範您不信任的指令碼。如果變更執行原則，可能會使您接觸到 about_Execution_Policies 說明主題 (網址為
https:/go.microsoft.com/fwlink/?LinkID=135170) 中所述的安全性風險。您要變更執行原則嗎?
[Y] 是(Y)  [A] 全部皆是(A)  [N] 否(N)  [L] 全部皆否(L)  [S] 暫停(S)  [?] 說明 (預設值為 "N"): Y
PS C:\Windows\system32>

```

- 取得電腦ip (我的電腦是192.168.0.58)
```
PS C:\> ipconfig

Windows IP 設定


乙太網路卡 乙太網路:

   連線特定 DNS 尾碼 . . . . . . . . :
   連結-本機 IPv6 位址 . . . . . . . : fe80::b45f:e061:7d35:814%3
   IPv4 位址 . . . . . . . . . . . . : 192.168.0.58
   子網路遮罩 . . . . . . . . . . . .: 255.255.255.0
   預設閘道 . . . . . . . . . . . . .: 192.168.0.1
```
 - 切換到下載的mini-auto-main裡
```
cd C:\Users\Hcedu\Documents\PAUL\mini-auto-main
```
- 安裝必要套件
```
PS C:\Users\Hcedu\Documents\PAUL\mini-auto-main> pip install -r .\requirements.txt
Collecting fastapi (from -r .\requirements.txt (line 1))
  Downloading fastapi-0.116.1-py3-none-any.whl.metadata (28 kB)
Collecting uvicorn (from -r .\requirements.txt (line 2))
  Downloading uvicorn-0.35.0-py3-none-any.whl.metadata (6.5 kB)
  ...
  ...
  ...
rlette-0.47.2 typing-extensions-4.14.1 typing-inspection-0.4.1 uvicorn-0.35.0

[notice] A new release of pip is available: 24.3.1 -> 25.1.1
[notice] To update, run: python.exe -m pip install --upgrade pip
```

 - 執行後端主程式
```
PS C:\Users\Hcedu\Documents\PAUL\mini-auto-main> python .\main.py
C:\Users\Hcedu\Documents\PAUL\mini-auto-main\main.py:28: DeprecationWarning:
        on_event is deprecated, use lifespan event handlers instead.

        Read more about it in the
        [FastAPI docs for Lifespan Events](https://fastapi.tiangolo.com/advanced/events/).

  @app.on_event("startup")
C:\Users\Hcedu\Documents\PAUL\mini-auto-main\main.py:39: DeprecationWarning:
        on_event is deprecated, use lifespan event handlers instead.

        Read more about it in the
        [FastAPI docs for Lifespan Events](https://fastapi.tiangolo.com/advanced/events/).

  @app.on_event("shutdown")
[32mINFO[0m:     Started server process [[36m580[0m]
[32mINFO[0m:     Waiting for application startup.
[32mINFO[0m:     Application startup complete.
[32mINFO[0m:     Uvicorn running on [1mhttp://0.0.0.0:8000[0m (Press CTRL+C to quit)
```
- 允許防火牆
- 開啟chrome測試Web server是否啟動
```
http://192.168.0.58:8000
```

- 另開powershell, 執行測試程式
```
PS C:\Users\Hcedu> cd C:\Users\Hcedu\Documents\PAUL\mini-auto-main\test\feature
PS C:\Users\Hcedu\Documents\PAUL\mini-auto-main\test\feature> .\test_get_latest_data.ps1

--- Setting up data for GET /api/latest_data ---
Setting manual control data...


message : Manual control command received




Setting control mode...


message : Control mode set to autonomous




Registering camera IP...


message : ESP32-S3 IP registered successfully




Sending sync data...


c : 0
m : 0
d : 0
a : 90




--- Testing GET /api/latest_data ---


latest_data          : @{s=100; v=800; t=System.Object[]; i=192.168.1.200; u=; m=; d=; a=; c=}
latest_command       : @{c=0; m=0; d=0; a=90}
esp32_cam_ip         : 192.168.1.200
current_control_mode : autonomous
thermal_analysis     : @{max_temp=4.0; min_temp=1.0; avg_temp=2.5; hotspot_detected=False}
visual_analysis      :




--- Test Complete ---
PS C:\Users\Hcedu\Documents\PAUL\mini-auto-main\test\feature>
```
