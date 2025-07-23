# apis/status.py
# 這個模組定義了處理車輛狀態更新的 API 端點。

from fastapi import APIRouter # 導入 APIRouter，用於創建和管理 API 路由。
from pydantic import BaseModel # 導入 BaseModel，用於定義資料模型，實現資料驗證和序列化。

# 創建一個 Pydantic 模型 VehicleStatus，用於定義車輛狀態更新的資料結構。
class VehicleStatus(BaseModel):
    battery: int  # 電池電量，例如：98 (表示 98%)
    current_state: str  # 當前狀態，例如："stopped" (停止), "moving_forward" (前進)

# 創建一個 API 路由實例，用於處理與狀態相關的請求。
router = APIRouter()

# 定義一個 POST 請求的 API 端點：/status。
# 當車輛（例如 Arduino）發送狀態更新時，會呼叫此函數。
@router.post("/status")
def update_vehicle_status(status: VehicleStatus):
    """
    接收來自車輛（例如 Arduino）的狀態更新。
    """
    # 目前，我們僅將接收到的資料列印到伺服器控制台。
    # 在實際應用中，您可能會將這些資料儲存到資料庫、更新儀表板或觸發其他邏輯。
    print(f"[狀態更新接收] 電池: {status.battery}%, 狀態: {status.current_state}")
    
    # 確認已成功接收狀態更新。
    return {"message": "狀態更新成功", "received_data": status}
