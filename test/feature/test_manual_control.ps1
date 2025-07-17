$SERVER_URL = "http://127.0.0.1:8000"

# 1. Set control mode to MANUAL
Write-Host "`n--- Setting control mode to MANUAL ---"
$body_set_control_mode_manual = @{
    mode = "manual"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/set_control_mode" -Method POST -ContentType "application/json" -Body $body_set_control_mode_manual | Format-List

# 2. Send a sync request with manual control commands
Write-Host "`n--- Sending manual control commands via /api/sync ---"
$body_sync_manual_control = @{
    s = 1
    v = 7800
    m = 150
    d = 90
    a = 45
    c = 1
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/sync" -Method POST -ContentType "application/json" -Body $body_sync_manual_control | Format-List

Write-Host "`n--- Test Complete ---"
