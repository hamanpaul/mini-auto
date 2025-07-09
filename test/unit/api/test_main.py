# tests/test_main.py
import sys
import os
from fastapi.testclient import TestClient

# Add the project root to the Python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'src', 'py_rear')))

from src.py_rear.main import app

# 創建測試客戶端
# Create a test client
client = TestClient(app)

def test_read_root():
    # 測試根路徑是否返回預期的歡迎訊息
    # Test if the root path returns the expected welcome message
    response = client.get("/")
    assert response.status_code == 200
    assert response.json() == {"message": "歡迎使用 FastAPI 伺服器"}
