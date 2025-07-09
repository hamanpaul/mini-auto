#!/bin/bash

# Test POST /api/sync with thermal data

SERVER_URL="http://127.0.0.1:8000"

echo "
--- Testing POST /api/sync (with thermal data) ---"
curl -X POST ${SERVER_URL}/api/sync \
     -H "Content-Type: application/json" \
     -d '{"s": 79, "v": 785, "t": [[2550, 2600], [2575, 2625]], "i": "192.168.1.200"}'

echo "
--- Test Complete ---"