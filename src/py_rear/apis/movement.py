# apis/movement.py
from fastapi import APIRouter

# 創建一個 API 路由，用於處理移動相關的請求
# Create an API router for movement-related requests
router = APIRouter()

# 定義前進的端點
# Define the forward endpoint
@router.get("/forward")
def move_forward():
    # 返回前進的訊息
    # Return a forward message
    return {"action": "前進", "message": "正在向前移動"}

# 定義後退的端點
# Define the backward endpoint
@router.get("/backward")
def move_backward():
    # 返回後退的訊息
    # Return a backward message
    return {"action": "後退", "message": "正在向後移動"}

# 定義左轉的端點
# Define the turn left endpoint
@router.get("/turn_left")
def turn_left():
    # 返回左轉的訊息
    # Return a turn left message
    return {"action": "左轉", "message": "正在向左轉"}

# 定義右轉的端點
# Define the turn right endpoint
@router.get("/turn_right")
def turn_right():
    # 返回右轉的訊息
    # Return a turn right message
    return {"action": "右轉", "message": "正在向右轉"}
