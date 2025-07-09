#!/bin/bash

# Test POST /api/manual_control

SERVER_URL="http://127.0.0.1:8000"

echo "
--- Testing POST /api/manual_control ---"
curl -X POST ${SERVER_URL}/api/manual_control \
     -H "Content-Type: application/json" \
     -d '{"m": 50, "d": 90, "a": 45, "c": 1}'

echo "
--- Test Complete ---"