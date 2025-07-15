API 文件範例
這份文件描述了與裝置互動的 JSON-RPC API。

Login_API
描述
登入並獲取一個 session ID，用於後續的 API 請求認證。

請求範例
curl -X POST -d '{ "jsonrpc": "2.0", "id": 1, "method": "call", "params": [ "00000000000000000000000000000000", "session", "login", { "username": "admin", "password": "xxxxxxxx" } ] }' [https://viasatrouter.com/ubus](https://viasatrouter.com/ubus) -k


成功回應
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": [
    0,
    {
      "ubus_rpc_session": "ba255e5093ff32016d3df27f0b82f3ab",
      "timeout": 300,
      "expires": 300,
      "acls": {
        "access-group": {
          "api": [
            "read"
          ],
          "unauthenticated": [
            "read"
          ]
        },
        "ubus": {
          "networking.*": [
            "*"
          ],
          "router.*": [
            "*"
          ],
          "rpc-sys": [
            "*"
          ],
          "session": [
            "access",
            "login"
          ],
          "wireless.*": [
            "*"
          ]
        }
      },
      "data": {
        "username": "admin"
      }
    }
  ]
}


參數說明
|

| 參數 | 類型 | 說明 |
| Request |  |  |
| username | String | 使用者名稱，與圖形介面 (GUI) 的使用者名稱相同。 |
| password | String | 使用者密碼，與圖形介面 (GUI) 的密碼相同。 |
| timeout | Integer | Session 過期時間，單位為秒，預設為 300 秒 (可選)。 |
| Response |  |  |
| ubus_rpc_session | String | 用於後續請求的 Session ID。 |

WIFI_Restart_API
描述
重新啟動 WiFi 服務。有些 WiFi 相關的設定變更後需要呼叫此 API 才會生效。

請求範例
curl -X POST -d '{ "jsonrpc": "2.0", "id": 1, "method": "call", "params": [ "ba255e5093ff32016d3df27f0b82f3ab", "rpc-sys", "wifi_restart", {} ] }' [https://viasatrouter.com/ubus](https://viasatrouter.com/ubus) -k


成功回應
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": [
    0
  ]
}


參數說明
此 API 不需要任何參數。

WiFi_Settings_API
c) 設定 AP 組態功能
描述
設定無線基地台 (AP) 模式下的 WiFi 組態。

備註
此功能需要呼叫 WIFI_Restart_API 才能啟用變更。

請求範例
curl -X POST -d '{ "jsonrpc": "2.0", "id": 1, "method": "call", "params": [ "ba255e5093ff32016d3df27f0b82f3ab", "wireless.interface", "set", { "wifidev": "wifi0", "ssid": "viasat-5g-112233", "encryption": "psk-mixed", "key": "12345678", "ssid_broadcast": 1, "vid": 0 } ] }' [https://viasatrouter.com/ubus](https://viasatrouter.com/ubus) -k


成功回應
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": [
    0,
    {}
  ]
}


參數說明
| 參數 | 類型 | 說明 |
| Request |  |  |
| wifidev | String | 無線頻段 ("wifi0": 5GHz, "wifi1": 2.4GHz)。 |
| ssid | String | 網路名稱 (SSID)。 |
| encryption | String | 安全模式 (例如: "psk-mixed", "psk2+ccmp", "none")。 |
| key | String | WiFi 密碼。 |
| ssid_broadcast | Integer | SSID 廣播 (1: 啟用, 0: 停用)。 |
| vid | Integer | VLAN ID (0 表示此 SSID 未設定 VLAN)。 |

Router_Status_API
a) 獲取路由器狀態功能
描述
獲取路由器平台狀態資訊，如系統時間、版本號和 MAC 位址。

請求範例
curl -X POST -d '{ "jsonrpc": "2.0", "id": 1, "method": "call", "params": [ "ba255e5093ff32016d3df27f0b82f3ab", "router.info", "status", {} ] }'  [https://viasatrouter.com/ubus](https://viasatrouter.com/ubus) -k


成功回應
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": [
    0,
    {
      "sys_time": "Thu, 14 Oct 2020 02:08:29 UTC",
      "serial_num": "xxxxxxxxxx",
      "part_num": "xxxxxxxxxxx",
      "model_num": "RG3210",
      "sw_ver": "11.3.0.2.11b",
      "hw_ver": "0A",
      "mac_addr": "00:03:7F:BA:DB:AD"
    }
  ]
}


參數說明
| 參數 | 類型 | 說明 |
| Response |  |  |
| sys_time | String | 系統時間 |
| serial_num | String |  |