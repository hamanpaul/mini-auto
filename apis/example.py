# apis/example.py
from fastapi import APIRouter

# 创建一个 API 路由
# Create an API router
router = APIRouter()

# 定义一个 GET 请求的端点
# Define an endpoint for GET requests
@router.get("/hello")
def say_hello():
    # 返回一个 JSON 响应
    # Return a JSON response
    return {"message": "你好，世界！"}
