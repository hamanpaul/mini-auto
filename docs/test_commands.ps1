$SERVER_URL = "http://127.0.0.1:8000"

Write-Host "`n--- Testing API Endpoints ---"

# 1. Test POST /api/sync (with thermal data)
Write-Host "`nTesting POST /api/sync (with thermal data)..."
$body_sync_thermal = @{
    s = 79
    v = 785
    t = @((2550, 2600), (2575, 2625))
    i = "192.168.1.200"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/sync" -Method POST -ContentType "application/json" -Body $body_sync_thermal | Format-List

# 1. Test POST /api/sync (without thermal data)
Write-Host "`nTesting POST /api/sync (without thermal data)..."
$body_sync_no_thermal = @{
    s = 78
    v = 750
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/sync" -Method POST -ContentType "application/json" -Body $body_sync_no_thermal | Format-List

# 2. Test POST /api/register_camera
Write-Host "`nTesting POST /api/register_camera..."
$body_register_camera = @{
    i = "192.168.1.100"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/register_camera" -Method POST -ContentType "application/json" -Body $body_register_camera | Format-List

# 3. Test POST /api/manual_control
Write-Host "`nTesting POST /api/manual_control..."
$body_manual_control = @{
    m = 50
    d = 90
    a = 45
    c = 1
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/manual_control" -Method POST -ContentType "application/json" -Body $body_manual_control | Format-List

# 4. Test POST /api/set_control_mode (avoidance)
Write-Host "`nTesting POST /api/set_control_mode (avoidance)..."
$body_set_control_avoidance = @{
    mode = "avoidance"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/set_control_mode" -Method POST -ContentType "application/json" -Body $body_set_control_avoidance | Format-List

# 4. Test POST /api/set_control_mode (manual)
Write-Host "`nTesting POST /api/set_control_mode (manual)..."
$body_set_control_manual = @{
    mode = "manual"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/set_control_mode" -Method POST -ContentType "application/json" -Body $body_set_control_manual | Format-List

# 5. Test GET /api/latest_data
Write-Host "`nTesting GET /api/latest_data..."
Invoke-RestMethod -Uri "$SERVER_URL/api/latest_data" -Method GET | Format-List

Write-Host "`n--- API Testing Complete ---"
