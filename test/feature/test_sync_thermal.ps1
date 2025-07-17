$SERVER_URL = "http://127.0.0.1:8000"

Write-Host "`n--- Testing POST /api/sync (with thermal data) ---"
$body_sync_thermal = @{
    s = 79
    v = 785
    t = @((2550, 2600), (2575, 2625))
    i = "192.168.1.200"
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/sync" -Method POST -ContentType "application/json" -Body $body_sync_thermal | Format-List

Write-Host "`n--- Test Complete ---"
