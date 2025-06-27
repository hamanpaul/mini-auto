# main.py
import uvicorn
from fastapi import FastAPI
import os
import importlib

# Create a FastAPI app instance
app = FastAPI()

# Dynamically import all routes from the apis directory
apis_dir = "apis"
for filename in os.listdir(apis_dir):
    if filename.endswith(".py") and filename != "__init__.py":
        module_name = f"{apis_dir}.{filename[:-3]}"
        # Import the module
        module = importlib.import_module(module_name)
        # Register the router
        app.include_router(module.router)

# Define the root path
@app.get("/")
def read_root():
    # Return a simple welcome message
    return {"message": "歡迎使用 FastAPI 伺服器"}

# Run the server
if __name__ == "__main__":
    # Use uvicorn to run the FastAPI app
    uvicorn.run(app, host="0.0.0.0", port=8000)
