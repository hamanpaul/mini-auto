from fastapi import APIRouter

router = APIRouter()

@router.post("/api/discovery/connected")
async def arduino_connected():
    print("Received Arduino connected signal. (Broadcast stopping handled by main.py)")
    return {"message": "Arduino connected signal received."}

