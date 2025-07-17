$SERVER_URL = "http://127.0.0.1:8000"
$ESP32_IP = "192.168.1.105"

Write-Host "發送 POST 請求到 $SERVER_URL/api/register_camera"

$body_register_camera = @{
    i = $ESP32_IP
} | ConvertTo-Json

Write-Host "酬載: $body_register_camera"

Invoke-RestMethod -Uri "$SERVER_URL/api/register_camera" -Method POST -ContentType "application/json" -Body $body_register_camera | Format-List
