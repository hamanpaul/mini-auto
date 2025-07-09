#!/bin/bash

# Test POST /api/sync without thermal data

SERVER_URL="http://127.0.0.1:8000"

echo "
--- Testing POST /api/sync (without thermal data) ---"
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{"s": 78, "v": 750}'

echo "
--- Test Complete ---"