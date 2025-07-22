from fastapi import APIRouter, HTTPException, Response
from fastapi.responses import StreamingResponse
import io
import cv2
import base64

# This will be set by main.py during startup
camera_processor = None

router = APIRouter()

@router.post("/camera/start")
async def start_camera_stream():
    if camera_processor is None:
        raise HTTPException(status_code=500, detail="Camera processor not initialized.")
    if camera_processor.is_running():
        return {"message": "Camera stream processor is already running.", "status": "running"}
    
    camera_processor.start()
    return {"message": "Camera stream processor started.", "status": "starting"}

@router.post("/camera/stop")
async def stop_camera_stream():
    if camera_processor is None:
        raise HTTPException(status_code=500, detail="Camera processor not initialized.")
    if not camera_processor.is_running():
        return {"message": "Camera stream processor is not running.", "status": "stopped"}
    
    camera_processor.stop()
    return {"message": "Camera stream processor stopped.", "status": "stopping"}

@router.get("/camera/status")
async def get_camera_status():
    if camera_processor is None:
        return {"status": "not_initialized"}
    return {"status": "running" if camera_processor.is_running() else "stopped"}

@router.get("/camera/analysis")
async def get_visual_analysis():
    """Returns the latest visual analysis results from the processor."""
    if camera_processor is None or not camera_processor.is_running():
        raise HTTPException(status_code=404, detail="Camera analysis not running.")
    
    _, _, analysis_results = camera_processor.get_latest_frame()
    
    if analysis_results is None:
        raise HTTPException(status_code=404, detail="No analysis results available.")
    
    return analysis_results
