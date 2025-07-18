## Miniauto 專案待辦事項 (TODO List)

本文件列出了 Miniauto 專案未來需要開發和改進的功能與任務。

### 1. Arduino UNO 韌體

*   **感測器整合**：
    *   **已完成**：重新引入 HC-SR04 (I2C variant) 超音波感測器功能，並透過現有 `Ultrasound` 函式庫實現，經評估對記憶體影響極小。
*   **致動器控制**：
    *   **已完成**：優化 `controlBuzzer` 函式，使其能更精確地控制蜂鳴器持續時間，而不是依賴 `delay`。
*   **錯誤處理與穩健性**：
    *   **已完成**：實現更精細的錯誤恢復機制，而不僅僅是 `ESP.restart()`。
*   **通訊**：
    *   **部分完成**：考慮實現更穩健的 AT Command 錯誤處理和重試機制。
        *   **未完成的部分**：缺乏明確的 AT Command 錯誤重試機制。當 AT 命令失敗時，目前只是記錄錯誤或關閉連接，沒有嘗試重新發送命令或重新初始化模組。
*   **週邊功能驗證**：
    *   **待驗證**: 馬達控制，超音波偵測，熱成像，wifi模組連線(需重設計)

### 2. ESP32-CAM 韌體

*   **錯誤處理與穩健性**：
    *   **未完成**：實現更精細的錯誤恢復機制，而不僅僅是 `ESP.restart()`。
*   **相機控制**：
    *   **未完成**：增加相機參數的動態調整功能（例如亮度、對比度、飽和度等），可透過後端 API 控制。
*   **影像處理**：
    *   **未完成**：考慮在 ESP32 端進行初步的影像處理或壓縮，以減少 Wi-Fi 傳輸負載（例如，邊緣檢測、ROI 裁剪）。

### 3. Python FastAPI 後端

*   **視覺分析**：
    *   **部分完成**：實現更複雜的視覺分析演算法（例如，基於深度學習的目標識別、追蹤、車道線檢測）。
        *   **未完成的部分**：尚未實現基於深度學習的目標識別、追蹤、車道線檢測等更複雜的演算法。現有邏輯的優化空間也很大。
*   **數據管理**：
    *   **未完成**：增加數據持久化儲存功能，將歷史感測器數據、控制指令和分析結果儲存到資料庫（例如 SQLite, PostgreSQL）。
    *   **未完成**：實現數據查詢和報告功能。
*   **使用者介面 (UI)**：
    *   **部分完成**：開發一個完整的 Web UI 介面，用於實時監控車輛狀態、控制車輛、顯示影像串流和熱成像數據。
        *   **未完成的部分**：需要開發一個功能齊全的 Web UI，包括實時監控、車輛控制、影像串流、熱成像數據顯示以及歷史數據的視覺化圖表。
*   **日誌系統**：
    *   **部分完成**：實現更完善的日誌系統，支援日誌級別（DEBUG, INFO, WARNING, ERROR）、日誌輪轉和輸出到檔案。
        *   **未完成的部分**：需要實現日誌輪轉、將日誌輸出到檔案，並可能需要整合標準的 Python 日誌模組以支援更細緻的日誌級別控制。
*   **安全**：
    *   **未完成**：增加認證和授權機制，保護 API 端點。
*   **控制邏輯**：
    *   **部分完成**：優化 `_generate_avoidance_commands` 和 `_generate_autonomous_commands` 中的決策邏輯，使其更智慧、更適應複雜環境。
        *   **未完成的部分**：需要優化決策邏輯，使其更智慧、更適應複雜環境，並引入 PID 控制器或其他控制演算法以實現更精確的馬達和舵機控制。

### 4. 整體系統

*   **測試**：
    *   **部分完成**：開發全面的單元測試和整合測試，確保各個模組和系統的穩定性。
        *   **未完成的部分**：需要評估現有測試的覆蓋率和全面性，並根據需要增加更多的單元測試和整合測試。
*   **部署**：
    *   **未完成**：建立 CI/CD (持續整合/持續部署) 流程，自動化測試和部署。
*   **文件**：
    *   **部分完成**：撰寫更詳細的硬體組裝指南和接線圖。
        *   **未完成的部分**：需要審查現有文件，確保其詳細程度和完整性，特別是硬體組裝指南和接線圖，並建立 API 文件的持續更新機制。
    *   **部分完成**：更新和維護 API 文件。
        *   **未完成的部分**：需要建立 API 文件的持續更新機制。
*   **性能優化**：
    *   **未完成**：對整個系統進行性能分析和優化，特別是通訊延遲和影像處理速度。
