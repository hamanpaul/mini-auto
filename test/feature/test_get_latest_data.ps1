$SERVER_URL = "http://127.0.0.1:8000"

Write-Host "`n--- Setting up data for GET /api/latest_data ---"

# 1. Set manual control data
Write-Host "Setting manual control data..."
$body_manual_control = @{
    m = 60
    d = 180
    a = 70
    c = 2
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/manual_control" -Method POST -ContentType "application/json" -Body $body_manual_control | Format-List

# 2. Set control mode
Write-Host "`nSetting control mode..."
$body_set_control_mode = @{
    mode = "autonomous"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/set_control_mode" -Method POST -ContentType "application/json" -Body $body_set_control_mode | Format-List

# 3. Register camera IP
Write-Host "`nRegistering camera IP..."
$body_register_camera = @{
    i = "192.168.1.200"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/register_camera" -Method POST -ContentType "application/json" -Body $body_register_camera | Format-List

# 4. Send sync data to populate latest_arduino_data and latest_command_sent
Write-Host "`nSending sync data..."
$body_sync_data = @{
    s = 100
    v = 800
    t = @((100, 200), (300, 400))
    i = "192.168.1.200"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/sync" -Method POST -ContentType "application/json" -Body $body_sync_data | Format-List

Write-Host "`n--- Testing GET /api/latest_data ---"
Invoke-RestMethod -Uri "$SERVER_URL/api/latest_data" -Method GET | Format-List

Write-Host "`n--- Test Complete ---"
