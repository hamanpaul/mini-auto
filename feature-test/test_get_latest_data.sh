#!/bin/bash

# Test GET /api/latest_data

SERVER_URL="http://127.0.0.1:8000"

# Function to start the FastAPI server
start_server() {
    echo "Starting FastAPI server..."
    python main.py > server.log 2>&1 &
    SERVER_PID=$!
    echo "Server PID: $SERVER_PID"
    sleep 5
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
    sleep 2
    if ! lsof -ti :8000; then
        echo "FastAPI server stopped successfully."
    else
        echo "Warning: FastAPI server did not stop gracefully. Attempting to force kill."
        kill -9 $SERVER_PID
        sleep 2
        if ! lsof -ti :8000; then
            echo "FastAPI server force-stopped successfully."
        else
            echo "Error: FastAPI server still did not stop after force kill."
            exit 1
        fi
    fi
    rm server.log
}

trap stop_server EXIT

start_server

echo "
--- Setting up data for GET /api/latest_data ---"

# 1. Set manual control data
curl -X POST ${SERVER_URL}/api/manual_control \
     -H "Content-Type: application/json" \
     -d '{"m": 60, "d": 180, "a": 70, "c": 2}'

# 2. Set control mode
curl -X POST ${SERVER_URL}/api/set_control_mode \
     -H "Content-Type: application/json" \
     -d '{"mode": "autonomous"}'

# 3. Register camera IP
curl -X POST ${SERVER_URL}/api/register_camera \
     -H "Content-Type: application/json" \
     -d '{"i": "192.168.1.200"}'

# 4. Send sync data to populate latest_arduino_data and latest_command_sent
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{"s": 100, "v": 800, "t": [[100, 200], [300, 400]], "i": "192.168.1.200"}'

echo "
--- Testing GET /api/latest_data ---"
curl -X GET ${SERVER_URL}/api/latest_data

echo "
--- Test Complete ---"
