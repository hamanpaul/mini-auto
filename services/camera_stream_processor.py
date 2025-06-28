import requests
import cv2
import numpy as np
import asyncio
import threading
import time

class CameraStreamProcessor:
    def __init__(self, esp32_cam_ip: str):
        self.esp32_cam_ip = esp32_cam_ip
        self.stream_url = f"http://{self.esp32_cam_ip}/stream"
        self._running = False
        self._thread = None
        self._latest_frame = None
        self._latest_processed_frame = None
        self._lock = threading.Lock() # For thread-safe access to frames

    def _get_mjpeg_stream(self):
        """
        Connects to an MJPEG stream and yields JPEG frames.
        This method runs in a separate thread.
        """
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
                    jpg_start = bytes_data.find(b'

', a) + 4
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
                # Example: Convert to grayscale and detect edges
                gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                edges = cv2.Canny(gray_frame, 100, 200)
                
                # You can store different processed results as needed
                with self._lock:
                    self._latest_processed_frame = edges # Store the processed frame (e.g., edges)
            else:
                print("Failed to decode frame for processing.")

        except Exception as e:
            print(f"Error during OpenCV frame processing: {e}")

    def start(self):
        if not self._running:
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
            return self._latest_frame, self._latest_processed_frame

    def is_running(self):
        return self._running

# Example Usage (for testing this module independently)
if __name__ == "__main__":
    # IMPORTANT: Replace with your ESP32-S3-CAM's actual IP address
    # You can find this in the Arduino Serial Monitor output
    ESP32_CAM_IP = "YOUR_ESP32_CAM_IP_ADDRESS" # <<<<< CHANGE THIS

    if ESP32_CAM_IP == "YOUR_ESP32_CAM_IP_ADDRESS":
        print("ERROR: Please update ESP32_CAM_IP with your ESP32-S3-CAM's actual IP address.")
        print("You can find the IP address in the Arduino Serial Monitor after uploading the sketch.")
    else:
        processor = CameraStreamProcessor(ESP32_CAM_IP)
        processor.start()

        try:
            while True:
                raw_frame, processed_frame = processor.get_latest_frame()
                if processed_frame is not None:
                    # Display the processed frame (e.g., edges)
                    cv2.imshow("Processed Frame (Edges)", processed_frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break
                time.sleep(0.1) # Don't busy-wait too much
        except KeyboardInterrupt:
            print("Interrupted by user.")
        finally:
            processor.stop()
            cv2.destroyAllWindows()
            print("Exiting.")
