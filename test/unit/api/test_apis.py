# tests/test_apis.py
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

def test_say_hello():
    # 測試 /hello 路徑是否返回預期的訊息
    # Test if the /hello path returns the expected message
    response = client.get("/hello")
    assert response.status_code == 200
    assert response.json() == {"message": "你好，世界！"}
