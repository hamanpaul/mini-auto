import requests # 導入 requests 函式庫，用於發送 HTTP 請求。
import cv2 # 導入 OpenCV 函式庫，用於影像處理。
import numpy as np # 導入 NumPy 函式庫，用於數值運算。
import asyncio # 導入 asyncio 函式庫，用於非同步操作（雖然在此檔案中未直接使用，但通常與非同步程式碼相關）。
import threading # 導入 threading 函式庫，用於多執行緒編程。
import time # 導入 time 函式庫，用於時間相關操作。

class CameraStreamProcessor:
    # 類別的初始化函數。
    def __init__(self):
        self.esp32_cam_ip = None # 儲存 ESP32-CAM 的 IP 位址。
        self.stream_url = None # 儲存影像串流的 URL。
        self._running = False # 標誌，指示串流處理器是否正在運行。
        self._thread = None # 儲存運行串流的執行緒實例。
        self._latest_frame = None # 儲存最新的原始影像幀（JPEG 位元組）。
        self._latest_processed_frame = None # 儲存最新的已處理影像幀（OpenCV 影像物件）。
        self._latest_visual_analysis_results = None # 儲存最新的視覺分析結果。
        self._lock = threading.Lock() # 創建一個執行緒鎖，用於安全地訪問共享資料（影像幀）。
        self._prev_frame_time = 0 # 用於計算 FPS 的前一幀時間。
        self._frame_count = 0 # 用於計算 FPS 的幀計數。
        self._fps = 0.0 # 儲存計算出的 FPS。

    # 更新影像串流來源的函數。
    def update_stream_source(self, esp32_cam_ip: str):
        # 如果新的 IP 位址與當前設定的相同，則無需更改。
        if self.esp32_cam_ip == esp32_cam_ip:
            print(f"串流 IP 已設定為 {esp32_cam_ip}。無需更改。")
            return

        self.esp32_cam_ip = esp32_cam_ip # 更新 ESP32-CAM 的 IP 位址。
        # 根據 IP 位址構建影像串流的 URL。
        self.stream_url = f"http://{self.esp32_cam_ip}/stream"
        print(f"相機串流來源已更新為: {self.stream_url}")

        # 如果串流正在運行，則停止並使用新的 IP 重新啟動。
        if self._running:
            print("串流正在運行，正在使用新 IP 重新啟動...")
            self.stop()
            self.start()

    # 內部函數，用於連接 MJPEG 串流並處理影像幀。此函數在單獨的執行緒中運行。
    def _get_mjpeg_stream(self):
        """
        使用 OpenCV 連接到 MJPEG 串流並處理影像幀。
        此方法在單獨的執行緒中運行。
        """
        # 如果串流 URL 未設定，則列印錯誤訊息並停止運行。
        if not self.stream_url:
            print("錯誤: 串流 URL 未設定。無法開始串流。")
            self._running = False
            return

        print(f"正在使用 OpenCV 連接到 MJPEG 串流: {self.stream_url}")
        cap = cv2.VideoCapture(self.stream_url) # 使用 OpenCV 創建視訊捕捉物件。

        # 如果無法打開視訊串流，則列印錯誤訊息並停止運行。
        if not cap.isOpened():
            print(f"錯誤: 無法打開視訊串流: {self.stream_url}")
            self._running = False
            return

        try:
            while self._running: # 迴圈運行，直到 _running 標誌變為 False。
                ret, frame = cap.read() # 從串流中讀取一幀影像。
                if not ret: # 如果無法讀取幀（串流結束或失敗）。
                    print(f"警告: 無法從串流中讀取幀。嘗試重新連接到 {self.stream_url}")
                    time.sleep(1) # 等待一小段時間，然後嘗試重新連接。
                    cap.release() # 釋放當前的視訊捕捉物件。
                    cap = cv2.VideoCapture(self.stream_url) # 重新創建視訊捕捉物件。
                    if not cap.isOpened(): # 如果重新連接失敗，則停止。
                        print(f"錯誤: 無法重新連接到串流 {self.stream_url}。正在停止。")
                        self._running = False # 確保停止運行
                        break
                    continue

                # 將影像幀編碼為 JPEG 位元組，用於儲存和潛在的代理。
                ret, buffer = cv2.imencode('.jpg', frame) # 將幀編碼為 JPEG 格式。
                if not ret: # 如果編碼失敗。
                    print("警告: 無法將幀編碼為 JPEG。跳過此幀。")
                    continue
                
                frame_bytes = buffer.tobytes() # 將緩衝區轉換為位元組。

                with self._lock: # 使用鎖來保護對 _latest_frame 的訪問。
                    self._latest_frame = frame_bytes # 儲存原始影像幀的位元組。

                # 處理原始影像幀（由 OpenCV 直接讀取）。
                self._process_frame(frame)

        except Exception as e: # 捕獲任何意外的異常。
            print(f"串流過程中發生意外錯誤: {e}")
        finally: # 無論是否發生異常，都會執行的程式碼塊。
            print("串流處理執行緒已停止。")
            cap.release() # 釋放視訊捕捉物件。
            self._running = False # 確保在退出時 _running 標誌為 False。

    # 處理單個 OpenCV 影像幀的函數。
    def _process_frame(self, frame: np.ndarray):
        """
        處理單個 OpenCV 影像幀。
        """
        try:
            if frame is not None: # 確保影像幀不為空。
                # --- 您的 OpenCV 影像分析邏輯在這裡 ---
                # 本地、低資源的障礙物偵測：亮度閾值化 + 輪廓偵測。
                
                # 將影像轉換為灰度圖。
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

                # 應用高斯模糊以減少雜訊。
                gray = cv2.GaussianBlur(gray, (5, 5), 0)

                # 定義感興趣區域 (ROI) - 專注於影像的下半部分，用於偵測近距離障礙物。
                height, width = gray.shape # 獲取灰度圖的高度和寬度。
                roi_start_row = int(height * 0.5) # 從影像中間開始。
                roi = gray[roi_start_row:height, 0:width] # 擷取 ROI 區域。

                # 自適應閾值化以偵測物體（假設物體比背景暗）。
                # 根據光照條件調整 blockSize 和 C。
                thresh = cv2.adaptiveThreshold(roi, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 11, 2)

                # 形態學操作以清理遮罩。
                kernel = np.ones((3,3),np.uint8) # 創建一個 3x3 的核心。
                thresh = cv2.erode(thresh, kernel, iterations = 1) # 腐蝕操作。
                thresh = cv2.dilate(thresh, kernel, iterations = 1) # 膨脹操作。

                # 在閾值化影像中查找輪廓。
                contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                obstacle_detected = False # 標誌，指示是否偵測到障礙物。
                obstacle_center_x = -1 # 障礙物中心點的 X 座標。
                obstacle_area_ratio = 0.0 # 障礙物面積佔 ROI 總面積的比例。
                
                if contours: # 如果偵測到輪廓。
                    # 找到最大的輪廓（潛在的障礙物）。
                    largest_contour = max(contours, key=cv2.contourArea) # 根據面積找到最大的輪廓。
                    area = cv2.contourArea(largest_contour) # 計算最大輪廓的面積。
                    
                    # 根據面積過濾（避免小雜訊）和位置（確保是相關的障礙物）。
                    # 根據相機視角和預期障礙物大小調整 min_area_threshold。
                    min_area_threshold = 1200 # 範例：根據新的解析度調整此值，可能需要進一步測試和微調。
                    if area > min_area_threshold: # 如果面積大於最小閾值。
                        obstacle_detected = True # 設置障礙物偵測標誌為 True。
                        M = cv2.moments(largest_contour) # 計算輪廓的矩。
                        if M["m00"] != 0: # 避免除以零。
                            obstacle_center_x = int(M["m10"] / M["m00"]) # 計算障礙物中心點的 X 座標（相對於 ROI）。
                            # 如果需要，轉換為全域幀的 X 座標：obstacle_center_x_global = obstacle_center_x
                        obstacle_area_ratio = round(area / (roi.shape[0] * roi.shape[1]), 4) # 計算障礙物面積比例。

                        # 可選：在原始影像幀上繪製輪廓以進行視覺化。
                        # cv2.drawContours(frame[roi_start_row:height, 0:width], [largest_contour], -1, (0, 255, 0), 2)

                # FPS 計算
                current_time = time.time()
                if self._prev_frame_time != 0:
                    time_diff = current_time - self._prev_frame_time
                    if time_diff > 0:
                        self._fps = 1.0 / time_diff
                    else:
                        self._fps = 0.0 # Avoid division by zero
                self._prev_frame_time = current_time

                with self._lock: # 使用鎖來保護對共享資料的訪問。
                    self._latest_processed_frame = frame # 儲存原始影像幀以供顯示。
                    self._latest_visual_analysis_results = { # 儲存視覺分析結果。
                        "obstacle_detected": obstacle_detected,
                        "obstacle_center_x": obstacle_center_x,
                        "obstacle_area_ratio": obstacle_area_ratio
                    }
            else:
                print("無法解碼幀以進行處理。")

        except Exception as e: # 捕獲 OpenCV 影像處理過程中可能發生的任何異常。
            print(f"OpenCV 影像處理過程中發生錯誤: {e}")

    # 啟動串流處理器的函數。
    def start(self):
        if not self._running: # 如果串流處理器未運行。
            if not self.stream_url: # 如果串流 URL 未設定。
                print("警告: 串流 URL 未設定。請先呼叫 update_stream_source()。")
                return
            print("正在啟動相機串流處理器...")
            self._running = True # 設置運行標誌為 True。
            self._thread = threading.Thread(target=self._get_mjpeg_stream) # 創建一個新執行緒來運行 _get_mjpeg_stream 函數。
            self._thread.daemon = True # 將執行緒設置為守護執行緒，允許主程式在執行緒運行時退出。
            self._thread.start() # 啟動執行緒。
            print("相機串流處理器已啟動。")
        else:
            print("相機串流處理器已在運行中。")

    # 停止串流處理器的函數。
    def stop(self):
        if self._running: # 如果串流處理器正在運行。
            print("正在停止相機串流處理器...")
            self._running = False # 設置運行標誌為 False，以停止執行緒。
            if self._thread and self._thread.is_alive(): # 如果執行緒存在且仍在運行。
                self._thread.join(timeout=5) # 等待執行緒完成，最多等待 5 秒。
                if self._thread.is_alive(): # 如果執行緒在超時後仍然在運行。
                    print("警告: 串流執行緒未能正常終止。")
            print("相機串流處理器已停止。")
        else:
            print("相機串流處理器未在運行中。")

    # 獲取最新影像幀和分析結果的函數。
    def get_latest_frame(self):
        """返回最新的原始影像幀（JPEG 位元組）及其處理後的版本（OpenCV 影像）。"""
        with self._lock: # 使用鎖來保護對共享資料的訪問。
            return self._latest_frame, self._latest_processed_frame, self._latest_visual_analysis_results # 返回最新的原始幀、處理後的幀和視覺分析結果。

    # 檢查串流處理器是否正在運行的函數。
    def is_running(self):
        return self._running # 返回 _running 標誌的當前狀態。

    # 獲取當前 FPS 的函數。
    def get_fps(self):
        with self._lock:
            return self._fps

# 範例用法（用於獨立測試此模組）。
if __name__ == "__main__":
    # 這部分將被移除或修改，因為 main.py 將管理實例。
    print("此模組旨在作為主 FastAPI 應用程式的一部分使用。")
    print("請運行 `python main.py` 來啟動應用程式。")
    # 原始測試程式碼已被註釋掉或移除，因為它與直接執行不再相關。
