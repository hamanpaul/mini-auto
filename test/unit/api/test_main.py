from fastapi.testclient import TestClient
from py_rear.main import app

# 創建測試客戶端
# Create a test client
client = TestClient(app)

def test_read_root():
    # 測試根路徑是否返回預期的歡迎訊息
    # Test if the root path returns the expected welcome message
    response = client.get("/")
    assert response.status_code == 200
    assert response.json() == {"message": "歡迎使用 FastAPI 伺服器"}

def test_dynamic_api_loading():
    # 測試動態載入的 API 是否正常運作
    # Test if the dynamically loaded API works correctly
    # 假設 example.py 中有一個 /hello 端點
    # Assume there is a /hello endpoint in example.py
    response = client.get("/hello")
    assert response.status_code == 200
    assert response.json() == {"message": "你好，世界！"}

    # 假設 movement.py 中有一個 /forward 端點
    # Assume there is a /forward endpoint in movement.py
    response = client.get("/forward")
    assert response.status_code == 200
    assert "status" in response.json()

    # 假設 status.py 中有一個 /status 端點 (POST)
    # Assume there is a /status endpoint in status.py (POST)
    response = client.post("/status", json={"battery": 99, "status": "testing"})
    assert response.status_code == 200
    assert "received_status" in response.json()
    assert response.json()["received_status"]["battery"] == 99
