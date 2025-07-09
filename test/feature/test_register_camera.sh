#!/bin/bash

# Test POST /api/register_camera

SERVER_URL="http://127.0.0.1:8000"

echo "
--- Testing POST /api/register_camera ---"
curl -X POST ${SERVER_URL}/api/register_camera \
     -H "Content-Type: application/json" \
     -d '{"i": "192.168.1.100"}'

echo "
--- Test Complete ---"