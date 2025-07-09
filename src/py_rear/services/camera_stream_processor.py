import requests
import cv2
import numpy as np
import asyncio
import threading
import time

class CameraStreamProcessor:
    def __init__(self):
        self.esp32_cam_ip = None
        self.stream_url = None
        self._running = False
        self._thread = None
        self._latest_frame = None
        self._latest_processed_frame = None
        self._latest_visual_analysis_results = None
        self._lock = threading.Lock() # For thread-safe access to frames

    def update_stream_source(self, esp32_cam_ip: str):
        if self.esp32_cam_ip == esp32_cam_ip:
            print(f"Stream IP already set to {esp32_cam_ip}. No change needed.")
            return

        self.esp32_cam_ip = esp32_cam_ip
        self.stream_url = f"http://{self.esp32_cam_ip}/stream"
        print(f"Camera stream source updated to: {self.stream_url}")

        if self._running:
            print("Stream is running, restarting with new IP...")
            self.stop()
            self.start()

    def _get_mjpeg_stream(self):
        """
        Connects to an MJPEG stream and yields JPEG frames.
        This method runs in a separate thread.
        """
        if not self.stream_url:
            print("Error: Stream URL not set. Cannot start streaming.")
            self._running = False
            return

        print(f"Connecting to MJPEG stream at: {self.stream_url}")
        try:
            response = requests.get(self.stream_url, stream=True, timeout=5)
            response.raise_for_status() # Raise an exception for HTTP errors
            
            boundary = None
            if 'content-type' in response.headers:
                content_type = response.headers['content-type']
                parts = content_type.split('boundary=')
                if len(parts) > 1:
                    boundary = parts[1].strip()
                    print(f"Found MJPEG boundary: {boundary}")
            
            if not boundary:
                print("Error: Could not find MJPEG boundary in Content-Type header.")
                self._running = False
                return

            bytes_data = b''
            for chunk in response.iter_content(chunk_size=8192):
                if not self._running: # Check if stop signal received
                    break
                bytes_data += chunk
                
                a = bytes_data.find(b'--' + boundary.encode('utf-8'))
                b = bytes_data.find(b'--' + boundary.encode('utf-8'), a + len(boundary) + 2)
                
                while a != -1 and b != -1:
                    jpg_start = bytes_data.find(b'\r\n\r\n', a) + 4
                    if jpg_start != -1 and jpg_start < b:
                        jpg_frame_bytes = bytes_data[jpg_start:b]
                        
                        with self._lock:
                            self._latest_frame = jpg_frame_bytes # Store raw frame
                        
                        self._process_frame(jpg_frame_bytes) # Process the frame
                        
                    bytes_data = bytes_data[b:]
                    a = bytes_data.find(b'--' + boundary.encode('utf-8'))
                    b = bytes_data.find(b'--' + boundary.encode('utf-8'), a + len(boundary) + 2)

        except requests.exceptions.RequestException as e:
            print(f"Stream request failed: {e}")
        except Exception as e:
            print(f"An unexpected error occurred during streaming: {e}")
        finally:
            print("Stream processing thread stopped.")
            self._running = False # Ensure running flag is false on exit

    def _process_frame(self, frame_bytes: bytes):
        """
        Decodes and processes a single JPEG frame using OpenCV.
        """
        try:
            np_array = np.frombuffer(frame_bytes, np.uint8)
            frame = cv2.imdecode(np_array, cv2.IMREAD_COLOR)

            if frame is not None:
                # --- Your OpenCV Image Analysis Logic Here ---
                # Local, low-resource obstacle detection: Brightness Thresholding + Contour Detection
                
                # Convert to grayscale
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

                # Apply Gaussian blur to reduce noise
                gray = cv2.GaussianBlur(gray, (5, 5), 0)

                # Define ROI (Region of Interest) - focus on the lower half of the image for close obstacles
                height, width = gray.shape
                roi_start_row = int(height * 0.5) # Start from middle of the image
                roi = gray[roi_start_row:height, 0:width]

                # Adaptive Thresholding to detect objects (assuming objects are darker than background)
                # Adjust blockSize and C based on lighting conditions
                thresh = cv2.adaptiveThreshold(roi, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 11, 2)

                # Morphological operations to clean up the mask
                kernel = np.ones((3,3),np.uint8)
                thresh = cv2.erode(thresh, kernel, iterations = 1)
                thresh = cv2.dilate(thresh, kernel, iterations = 1)

                # Find contours in the thresholded image
                contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                obstacle_detected = False
                obstacle_center_x = -1
                obstacle_area_ratio = 0.0
                
                if contours:
                    # Find the largest contour (potential obstacle)
                    largest_contour = max(contours, key=cv2.contourArea)
                    area = cv2.contourArea(largest_contour)
                    
                    # Filter by area (avoid small noise) and position (ensure it's a relevant obstacle)
                    # Adjust min_area_threshold based on camera view and expected obstacle size
                    min_area_threshold = 500 # Example: adjust this value
                    if area > min_area_threshold:
                        obstacle_detected = True
                        M = cv2.moments(largest_contour)
                        if M["m00"] != 0:
                            obstacle_center_x = int(M["m10"] / M["m00"]) # X-coordinate relative to ROI
                            # Convert to global frame X-coordinate if needed: obstacle_center_x_global = obstacle_center_x
                        obstacle_area_ratio = round(area / (roi.shape[0] * roi.shape[1]), 4)

                        # Optionally, draw the contour on the original frame for visualization
                        # cv2.drawContours(frame[roi_start_row:height, 0:width], [largest_contour], -1, (0, 255, 0), 2)

                with self._lock:
                    self._latest_processed_frame = frame # Store the original frame for display
                    self._latest_visual_analysis_results = {
                        "obstacle_detected": obstacle_detected,
                        "obstacle_center_x": obstacle_center_x,
                        "obstacle_area_ratio": obstacle_area_ratio
                    }
            else:
                print("Failed to decode frame for processing.")

        except Exception as e:
            print(f"Error during OpenCV frame processing: {e}")

    def start(self):
        if not self._running:
            if not self.stream_url:
                print("Warning: Stream URL not set. Call update_stream_source() first.")
                return
            print("Starting camera stream processor...")
            self._running = True
            self._thread = threading.Thread(target=self._get_mjpeg_stream)
            self._thread.daemon = True # Allow the main program to exit even if thread is running
            self._thread.start()
            print("Camera stream processor started.")
        else:
            print("Camera stream processor is already running.")

    def stop(self):
        if self._running:
            print("Stopping camera stream processor...")
            self._running = False
            if self._thread and self._thread.is_alive():
                self._thread.join(timeout=5) # Wait for the thread to finish
                if self._thread.is_alive():
                    print("Warning: Stream thread did not terminate gracefully.")
            print("Camera stream processor stopped.")
        else:
            print("Camera stream processor is not running.")

    def get_latest_frame(self):
        """Returns the latest raw frame (JPEG bytes) and its processed version (OpenCV image)."""
        with self._lock:
            return self._latest_frame, self._latest_processed_frame, self._latest_visual_analysis_results

    def is_running(self):
        return self._running

# Example Usage (for testing this module independently)
if __name__ == "__main__":
    # This part will be removed or modified as main.py will manage the instance
    print("This module is intended to be used as part of the main FastAPI application.")
    print("Please run `python main.py` to start the application.")
    # Original test code is commented out or removed as it's no longer relevant for direct execution.