$SERVER_URL = "http://127.0.0.1:8000"

Write-Host "`n--- Testing POST /api/sync (without thermal data) ---"
$body_sync_no_thermal = @{
    s = 78
    v = 750
} | ConvertTo-Json
Invoke-RestMethod -Uri "$SERVER_URL/api/sync" -Method POST -ContentType "application/json" -Body $body_sync_no_thermal | Format-List

Write-Host "`n--- Test Complete ---"
