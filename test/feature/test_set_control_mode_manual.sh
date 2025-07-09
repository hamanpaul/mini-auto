#!/bin/bash

# Test POST /api/set_control_mode (manual)

SERVER_URL="http://127.0.0.1:8000"

echo "
--- Testing POST /api/set_control_mode (manual) ---"
curl -X POST ${SERVER_URL}/api/set_control_mode \
     -H "Content-Type: application/json" \
     -d '{"mode": "manual"}'

echo "
--- Test Complete ---"