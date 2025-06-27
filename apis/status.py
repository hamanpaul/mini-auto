# apis/status.py
from fastapi import APIRouter
from pydantic import BaseModel

# Create a Pydantic model to define the structure of the status update
class VehicleStatus(BaseModel):
    battery: int  # e.g., 98
    current_state: str  # e.g., "stopped", "moving_forward"

# Create an API router for status-related requests
router = APIRouter()

@router.post("/status")
def update_vehicle_status(status: VehicleStatus):
    """
    Receives a status update from the vehicle (e.g., Arduino).
    """
    # For now, we'll just print the received data to the server console.
    # In a real application, you would save this to a database,
    # update a dashboard, or trigger other logic.
    print(f"[Status Update Received] Battery: {status.battery}%, State: {status.current_state}")
    
    # Acknowledge the reception
    return {"message": "Status updated successfully", "received_data": status}
