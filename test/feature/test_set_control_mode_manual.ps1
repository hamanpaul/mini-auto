$SERVER_URL = "http://127.0.0.1:8000"

Write-Host "`n--- Testing POST /api/set_control_mode (manual) ---"
$body_set_control_mode_manual = @{
    mode = "manual"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/set_control_mode" -Method POST -ContentType "application/json" -Body $body_set_control_mode_manual | Format-List

Write-Host "`n--- Test Complete ---"
