#!/bin/bash

# API Test Script using curl

SERVER_URL="http://127.0.0.1:8000"

# Function to start the FastAPI server
start_server() {
    echo "Starting FastAPI server..."
    python3 main.py > server.log 2>&1 &
    SERVER_PID=$!
    echo "Server PID: $SERVER_PID"
    # Wait for the server to start (adjust sleep time if needed)
    sleep 5
    # Check if server is actually listening
    if lsof -ti :8000; then
        echo "FastAPI server started successfully."
    else
        echo "Error: FastAPI server did not start."
        exit 1
    fi
}

# Function to stop the FastAPI server
stop_server() {
    echo "Stopping FastAPI server (PID: $SERVER_PID)..."
    kill $SERVER_PID
    # Give it a moment to shut down
    sleep 2
    if ! lsof -ti :8000; then
        echo "FastAPI server stopped successfully."
    else
        echo "Error: FastAPI server did not stop."
        exit 1
    fi
    rm server.log
}

# Ensure server is stopped on script exit
trap stop_server EXIT

# Start the server
start_server

echo "
--- Testing API Endpoints ---"

# 1. Test POST /api/sync
echo "
Testing POST /api/sync (with thermal data)..."
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{"s": 79, "v": 785, "t": [[2550, 2600], [2575, 2625]], "i": "192.168.1.200"}'

echo "
Testing POST /api/sync (without thermal data)..."
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{"s": 78, "v": 750}'

# 2. Test POST /api/register_camera
echo "
Testing POST /api/register_camera..."
curl -X POST ${SERVER_URL}/api/register_camera \
     -H "Content-Type: application/json" \
     -d '{"i": "192.168.1.100"}'

# 3. Test POST /api/manual_control
echo "
Testing POST /api/manual_control..."
curl -X POST ${SERVER_URL}/api/manual_control \
     -H "Content-Type: application/json" \
     -d '{"m": 50, "d": 90, "a": 45, "c": 1}'

# 4. Test POST /api/set_control_mode
echo "
Testing POST /api/set_control_mode (avoidance)..."
curl -X POST ${SERVER_URL}/api/set_control_mode \
     -H "Content-Type: application/json" \
     -d '{"mode": "avoidance"}'

echo "
Testing POST /api/set_control_mode (manual)..."
curl -X POST ${SERVER_URL}/api/set_control_mode \
     -H "Content-Type: application/json" \
     -d '{"mode": "manual"}'

# 5. Test GET /api/latest_data
echo "
Testing GET /api/latest_data..."
curl -X GET ${SERVER_URL}/api/latest_data

echo "
--- API Testing Complete ---"
