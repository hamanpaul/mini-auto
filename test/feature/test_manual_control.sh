#!/bin/bash

# Test manual control functionality via the /api/sync endpoint

SERVER_URL="http://127.0.0.1:8000"

# 1. Set control mode to MANUAL
echo "
--- Setting control mode to MANUAL ---"
curl -X POST ${SERVER_URL}/api/set_control_mode \
     -H "Content-Type: application/json" \
     -d '{"mode": "manual"}'

# 2. Send a sync request with manual control commands
echo "

--- Sending manual control commands via /api/sync ---"
# This simulates an Arduino sending its status and the client sending a command simultaneously
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{
          "s": 1, 
          "v": 7800, 
          "m": 150, 
          "d": 90, 
          "a": 45, 
          "c": 1
     }'

echo "

--- Test Complete ---"
