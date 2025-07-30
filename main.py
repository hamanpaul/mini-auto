# main.py
# 這是 FastAPI 應用程式的主入口點，負責啟動伺服器、載入 API 路由以及處理靜態檔案。

import uvicorn # 導入 uvicorn，這是一個 ASGI 伺服器，用於運行 FastAPI 應用程式。
from fastapi import FastAPI # 導入 FastAPI 類，用於創建 Web 應用程式。
from fastapi.middleware.cors import CORSMiddleware # 導入 CORSMiddleware，用於處理跨來源資源共享 (CORS)。
from fastapi.staticfiles import StaticFiles # 導入 StaticFiles，用於提供靜態檔案（如 CSS, JS, 圖片）。
from fastapi.responses import HTMLResponse # 導入 HTMLResponse，用於返回 HTML 內容。
import os # 導入 os 模組，用於與作業系統互動，例如處理檔案路徑。
import importlib # 導入 importlib 模組，用於動態導入其他 Python 模組。
import asyncio # 導入 asyncio 模組，用於非同步編程。
import sys # 導入 sys 模組，提供對 Python 解釋器相關變數和函數的訪問。
import subprocess # 導入 subprocess 模組，用於創建和管理子進程。
import logging # 導入 logging 模組，用於日誌記錄。

# 導入 CameraStreamProcessor 類，它負責處理來自 ESP32-CAM 的影像串流。
from src.py_rear.services.camera_stream_processor import CameraStreamProcessor
# 導入 camera API 模組，以便在啟動時設定其影像處理器實例。
from src.py_rear.apis import camera

# --- 配置 ---
# 這是 ESP32-S3-CAM 的 IP 位址。請務必將其替換為您實際 ESP32-S3-CAM 的 IP 位址。
# 您可以在 Arduino 序列埠監控器中找到這個 IP 位址。
ESP32_CAM_IP = "192.168.5.1" # <<<<< 這個 IP 位址通常來自 stream-server.html 或 ESP32-CAM 的輸出。

# 創建一個 FastAPI 應用程式實例。
app = FastAPI()
app.state.broadcast_process = None

# 配置 CORS 中介軟體，允許所有來源、憑證、方法和標頭。
# 在開發階段，可以設置 allow_origins=["*"] 來允許所有來源，方便測試。
# 在生產環境中，應將 allow_origins 設置為您的前端應用程式的具體來源，以提高安全性。
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # 允許所有來源
    allow_credentials=True, # 允許跨域請求攜帶憑證 (cookies, HTTP 認證等)
    allow_methods=["*"],  # 允許所有 HTTP 方法 (GET, POST, PUT, DELETE 等)
    allow_headers=["*"],  # 允許所有 HTTP 請求標頭
)

# 宣告一個全域變數，用於儲存 CameraStreamProcessor 的實例。初始值為 None。
camera_processor_instance: CameraStreamProcessor = None


# 使用 @app.on_event("startup") 裝飾器，定義一個在應用程式啟動時執行的非同步函數。
@app.on_event("startup")
async def startup_event():
    global camera_processor_instance
    # 在函數內部再次導入 camera 模組，確保在應用程式啟動時它已經可用。
    from src.py_rear.apis import camera as apis_camera
    # 創建 CameraStreamProcessor 的實例。
    camera_processor_instance = CameraStreamProcessor()
    # 將創建的實例賦值給 apis.camera 模組中的 camera_processor 變數，使其可在 API 路由中使用。
    apis_camera.camera_processor = camera_processor_instance

# 使用 @app.on_event("shutdown") 裝飾器，定義一個在應用程式關閉時執行的非同步函數。
@app.on_event("shutdown")
async def shutdown_event():
    # 如果 camera_processor_instance 存在且正在運行，則停止它。
    if camera_processor_instance and camera_processor_instance.is_running():
        camera_processor_instance.stop()
    
    # 停止廣播腳本。
    if app.state.broadcast_process:
        print("正在停止 IP 廣播腳本...")
        app.state.broadcast_process.terminate()
        app.state.broadcast_process.wait()
        print("廣播腳本已停止。")



# 動態導入 apis 目錄下的所有路由。
# 構建 apis 目錄的絕對路徑。
apis_dir = os.path.join(os.path.dirname(__file__), "src", "py_rear", "apis")
# 遍歷 apis 目錄中的所有檔案。
for filename in os.listdir(apis_dir):
    # 如果檔案以 .py 結尾且不是 __init__.py（避免重複導入或導入非路由檔案）。
    if filename.endswith(".py") and filename != "__init__.py":
        # 構建模組的完整名稱，例如 "src.py_rear.apis.camera"。
        module_name = f"src.py_rear.apis.{filename[:-3]}"
        # 動態導入該模組。
        module = importlib.import_module(module_name)
        # 將模組中定義的 APIRouter 包含到主 FastAPI 應用程式中，使其路由生效。
        app.include_router(module.router)

# 掛載靜態檔案。
# 將 'templates' 目錄下的檔案掛載到 'templates_static' 路徑下，使其可以透過 URL 訪問。
app.mount("/templates_static", StaticFiles(directory="templates"), name="templates_static")

# 定義根路徑 ("/") 的 GET 請求處理函數，用於提供 index.html 頁面。
@app.get("/", response_class=HTMLResponse)
async def read_root():
    # 打開並讀取 templates 目錄下的 index.html 檔案。
    with open(os.path.join(os.path.dirname(__file__), "templates", "index.html"), "r", encoding="utf-8") as f:
        return f.read() # 返回 HTML 內容。

# 判斷是否直接運行此腳本（而不是作為模組被導入）。
if __name__ == "__main__":
    # 檢查命令列參數，決定是否啟動廣播。
    if len(sys.argv) > 1 and sys.argv[1] == "b_ip":
        print("正在啟動 IP 廣播腳本...")
        app.state.broadcast_process = subprocess.Popen([sys.executable, "broadcast_server_ip.py"])
        print(f"廣播腳本已啟動，PID: {app.state.broadcast_process.pid}")

    # 使用 uvicorn 運行 FastAPI 應用程式。
    # host="0.0.0.0" 表示伺服器將監聽所有可用的網路介面，允許從外部訪問。
    # port=8000 表示伺服器將在 8000 埠上運行。

    # 配置日誌格式，包含時間戳
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

    uvicorn.run(app, host="0.0.0.0", port=8000, log_level="warning")
