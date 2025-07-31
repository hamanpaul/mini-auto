# 通報機制討論

本文檔記錄了關於 mini-auto 專案中火災警報通報機制的討論，包括本地通報與遠端通報的現有狀態及未來規劃。

## 1. 本地通報機制現有狀態

### 1.1 Arduino UNO 蜂鳴器

- **確認**: Arduino UNO 上有蜂鳴器 (`buzzerPin = 3`)，並有 `controlBuzzer(uint8_t buzzer_code)` 函式用於控制。
- **控制碼**: `0` (關閉), `1` (短蜂鳴), `2` (長蜂鳴), `3` (連續蜂鳴)。
- **現狀**: 蜂鳴器受控於從後端接收到的 `command_byte` 的低兩位。
- **問題**: 之前確認，後端在熱像儀偵測到危險時，**並未**將 `is_danger` 狀態反映到 `command_byte` 以觸發蜂鳴器。
- **已完成修改**: 已修改 `vehicle_api.py`，當 `latest_thermal_analysis_results.is_danger` 為 `True` 時，強制設定 `SyncResponse.c = 3` (連續蜂鳴)，使蜂鳴器與 GUI 警示同步。

### 1.2 GUI 警示

- **確認**: 前端 `index.html` 中已存在視覺警示機制。
- **觸發條件**: 當 Vue 實例的 `thermalDanger` 數據屬性為 `true` 時觸發。
- **顯示效果**: 顯示「危險！高溫！」文字，並應用 `blink-animation` 閃爍效果；同時 `body` 標籤會應用 `blink-background` class，使整個頁面背景紅黑閃爍。
- **數據來源**: `thermalDanger` 的值來自後端 `/api/latest_data` 返回的 `thermal_analysis.is_danger` 旗標。

## 2. 遠端通報機制討論

討論了多種遠端通報方案，以在火災警報時提供更廣泛的通知。

### 2.1 發送 Email (電子郵件)

- **原理**: 後端 Python 使用 `smtplib` 或第三方庫連接 SMTP 伺服器發送郵件。
- **優點**: 通用性高。
- **缺點**: 可能有延遲，需處理敏感帳密。

### 2.2 推送 Line Notify

- **原理**: 後端透過 HTTP POST 請求向 Line Notify API 發送訊息，需 Access Token。
- **優點**: 即時性高，台灣普及。
- **缺點**: 需使用者手動綁定並取得 Token。

### 2.3 發送 SMS (簡訊)

- **原理**: 整合第三方簡訊服務供應商 API。
- **優點**: 覆蓋面廣，無網路也能接收。
- **缺點**: 通常需付費，需註冊服務商帳號。

### 2.4 Android 手機推播 (FCM)

- **原理**: 透過 Google Firebase Cloud Messaging (FCM) 服務發送推播通知。
- **優點**: FCM 服務本身免費，即時性高。
- **缺點**: 需開發專用 Android App，增加開發複雜度。

### 2.5 LINE 機器人發送群組訊息 (選定方案)

- **原理**: 透過 LINE Messaging API，將機器人加入群組後，由後端直接控制發送訊息到群組。
- **優點**: **非常可行且相對簡單**，基本使用免費，可群組通知，API 簡單易用，使用者熟悉度高，即時性。
- **實作考量**: 需建立 LINE 機器人、取得 `Channel Access Token`、將機器人加入目標群組、取得群組 ID、後端整合 API 呼叫、安全儲存 Token。
- **現狀**: 已取得 `Channel Access Token`，下一步需獲取目標 LINE 群組 ID。

## 3. 後續計畫

- 關於 LINE 機器人獲取群組 ID 及發送測試訊息的實作，將在未來另開分支處理。
- 目前討論將重新聚焦於 `AUTONOMOUS` 模式的行為與流程設計。
