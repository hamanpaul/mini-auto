# 手動 API 測試指令
# Manual API Testing Commands

## 1. 啟動伺服器 (Start the Server)

在終端機中，進入專案根目錄 `/home/haman/python_api_server`，然後執行以下命令：
In your terminal, navigate to the project root directory `/home/haman/python_api_server` and run the following command:

```bash
python main.py
```

伺服器將會運行在 `http://localhost:8000`。
The server will be running at `http://localhost:8000`.

## 2. 測試指令 (Testing Commands)

在伺服器運行的狀態下，您可以使用 `curl` 或瀏覽器來測試以下端點。
While the server is running, you can use `curl` or a web browser to test the following endpoints.

---

### 根目錄 (Root)
- **說明 (Description):** 取得歡迎訊息 (Get the welcome message).
- **curl:**
  ```bash
  curl http://localhost:8000/
  ```
- **Browser URL:** `http://localhost:8000/`

---

### 打招呼 (Hello - from example API)
- **說明 (Description):** 取得 "你好，世界！" 的訊息 (Get the "Hello, World!" message).
- **curl:**
  ```bash
  curl http://localhost:8000/hello
  ```
- **Browser URL:** `http://localhost:8000/hello`

---

### 前進 (Forward)
- **說明 (Description):** 模擬向前移動 (Simulate moving forward).
- **curl:**
  ```bash
  curl http://localhost:8000/forward
  ```
- **Browser URL:** `http://localhost:8000/forward`

---

### 後退 (Backward)
- **說明 (Description):** 模擬向後移動 (Simulate moving backward).
- **curl:**
  ```bash
  curl http://localhost:8000/backward
  ```
- **Browser URL:** `http://localhost:8000/backward`

---

### 左轉 (Turn Left)
- **說明 (Description):** 模擬向左轉 (Simulate turning left).
- **curl:**
  ```bash
  curl http://localhost:8000/turn_left
  ```
- **Browser URL:** `http://localhost:8000/turn_left`

---

### 右轉 (Turn Right)
- **說明 (Description):** 模擬向右轉 (Simulate turning right).
- **curl:**
  ```bash
  curl http://localhost:8000/turn_right
  ```
- **Browser URL:** `http://localhost:8000/turn_right`
