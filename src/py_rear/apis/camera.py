from fastapi import APIRouter, HTTPException, Response # 導入 FastAPI 相關模組：APIRouter 用於定義路由，HTTPException 用於處理 HTTP 錯誤，Response 用於返回 HTTP 響應。
from fastapi.responses import StreamingResponse # 導入 StreamingResponse，用於處理串流響應（例如影像串流）。
import io # 導入 io 模組，用於處理位元組串流。
import cv2 # 導入 OpenCV 函式庫，用於影像處理。
import base64 # 導入 base64 模組，用於編碼和解碼資料。
import asyncio # 導入 asyncio 模組，用於非同步操作。

# camera_processor 變數將在 main.py 啟動時被設定為 CameraStreamProcessor 的實例。
camera_processor = None

# 創建一個 APIRouter 實例，用於定義與相機相關的 API 路由。
router = APIRouter()

# 定義一個 POST 請求的 API 端點：/camera/start，用於啟動相機串流處理器。
@router.post("/camera/start")
async def start_camera_stream():
    # 檢查 camera_processor 是否已初始化。如果為 None，表示服務未正確啟動，拋出 500 錯誤。
    if camera_processor is None:
        raise HTTPException(status_code=500, detail="Camera processor not initialized.")
    # 檢查相機串流處理器是否已經在運行。如果是，則返回相應訊息。
    if camera_processor.is_running():
        return {"message": "Camera stream processor is already running.", "status": "running"}
    
    # 啟動相機串流處理器。
    camera_processor.start()
    # 返回成功啟動的訊息。
    return {"message": "Camera stream processor started.", "status": "starting"}

# 定義一個 POST 請求的 API 端點：/camera/stop，用於停止相機串流處理器。
@router.post("/camera/stop")
async def stop_camera_stream():
    # 檢查 camera_processor 是否已初始化。
    if camera_processor is None:
        raise HTTPException(status_code=500, detail="Camera processor not initialized.")
    # 檢查相機串流處理器是否未在運行。如果是，則返回相應訊息。
    if not camera_processor.is_running():
        return {"message": "Camera stream processor is not running.", "status": "stopped"}
    
    # 停止相機串流處理器。
    camera_processor.stop()
    # 返回成功停止的訊息。
    return {"message": "Camera stream processor stopped.", "status": "stopping"}

# 定義一個 GET 請求的 API 端點：/camera/status，用於獲取相機串流處理器的運行狀態。
@router.get("/camera/status")
async def get_camera_status():
    # 檢查 camera_processor 是否已初始化。如果為 None，則返回未初始化狀態。
    if camera_processor is None:
        return {"status": "not_initialized"}
    # 根據 camera_processor 的運行狀態返回 "running" 或 "stopped"。
    return {"status": "running" if camera_processor.is_running() else "stopped"}

# 定義一個 GET 請求的 API 端點：/camera/analysis，用於獲取最新的視覺分析結果。
@router.get("/camera/analysis")
async def get_visual_analysis():
    """返回處理器最新的視覺分析結果。"""
    # 檢查 camera_processor 是否已初始化或是否正在運行。如果不是，則拋出 404 錯誤。
    if camera_processor is None or not camera_processor.is_running():
        raise HTTPException(status_code=404, detail="Camera analysis not running.")
    
    # 從 camera_processor 獲取最新的幀和分析結果。這裡只關心分析結果（第三個返回值）。
    _, _, analysis_results = camera_processor.get_latest_frame()
    
    # 如果沒有可用的分析結果，則拋出 404 錯誤。
    if analysis_results is None:
        raise HTTPException(status_code=404, detail="No analysis results available.")
    
    # 返回視覺分析結果。
    return analysis_results

@router.get("/camera/stream")
async def video_feed():
    """
    提供 MJPEG 影像串流。
    """
    if camera_processor is None or not camera_processor.is_running():
        raise HTTPException(status_code=404, detail="Camera stream not running.")

    async def generate_mjpeg_frames():
        while camera_processor.is_running():
            frame_bytes, _, _ = camera_processor.get_latest_frame()
            if frame_bytes:
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n'
                       b'Content-Length: ' + str(len(frame_bytes)).encode() + b'\r\n'
                       b'\r\n' + frame_bytes + b'\r\n')
            await asyncio.sleep(0.03) # 控制幀率，約 30 FPS

    return StreamingResponse(generate_mjpeg_frames(), media_type="multipart/x-mixed-replace; boundary=frame")
