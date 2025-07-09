# tests/test_movement.py
import sys
import os
from fastapi.testclient import TestClient

# Add the project root to the Python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'src', 'py_rear')))

from src.py_rear.main import app

# 創建測試客戶端
# Create a test client
client = TestClient(app)

def test_move_forward():
    # 測試 /forward 路徑
    # Test /forward path
    response = client.get("/forward")
    assert response.status_code == 200
    assert response.json() == {"action": "前進", "message": "正在向前移動"}

def test_move_backward():
    # 測試 /backward 路徑
    # Test /backward path
    response = client.get("/backward")
    assert response.status_code == 200
    assert response.json() == {"action": "後退", "message": "正在向後移動"}

def test_turn_left():
    # 測試 /turn_left 路徑
    # Test /turn_left path
    response = client.get("/turn_left")
    assert response.status_code == 200
    assert response.json() == {"action": "左轉", "message": "正在向左轉"}

def test_turn_right():
    # 測試 /turn_right 路徑
    # Test /turn_right path
    response = client.get("/turn_right")
    assert response.status_code == 200
    assert response.json() == {"action": "右轉", "message": "正在向右轉"}
