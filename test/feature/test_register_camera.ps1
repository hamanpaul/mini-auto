$SERVER_URL = "http://127.0.0.1:8000"

Write-Host "`n--- Testing POST /api/register_camera ---"
$body_register_camera = @{
    i = "192.168.1.100"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/register_camera" -Method POST -ContentType "application/json" -Body $body_register_camera | Format-List

Write-Host "`n--- Test Complete ---"
