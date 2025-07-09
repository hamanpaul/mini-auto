#!/bin/bash

# Test POST /api/sync without thermal data

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
--- Testing POST /api/sync (without thermal data) ---"
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{"s": 78, "v": 750}'

echo "
--- Test Complete ---"

