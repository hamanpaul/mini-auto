

/*
  Arduino API Client for Vehicle Control

  This sketch runs on an Arduino UNO and controls an ESP-01S module via AT commands
  to interact with a Python FastAPI server. It sends vehicle status updates and 
  receives movement commands.

  HARDWARE:
  - Arduino UNO
  - ESP-01S WiFi Module
  - AMG8833 Thermal Sensor
  - Motors, Servos, etc.

  LIBRARIES:
  - SoftwareSerial
  - Arduino_JSON

  SETUP:
  1. Connect ESP-01S TX to Arduino Pin 2 (RX) and RX to Arduino Pin 3 (TX).
  2. Update the `ssid`, `password`, and `serverIp` variables below.
  3. Open the Serial Monitor at 9600 baud to see debug output.
  4. Set the ESP-01S baud rate to 9600 if not already set.
*/

#include <SoftwareSerial.h>
#include <Arduino_JSON.h>

// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";         // Your WiFi network SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi network password
const char* serverIp = "192.168.1.100";      // The IP address of your computer running the Python server
const int serverPort = 8000;

// Define pins for SoftwareSerial to communicate with ESP-01S
// UNO RX = Pin 2, UNO TX = Pin 3
SoftwareSerial espSerial(2, 3); 

// --- Function Declarations ---
void setupEsp01s();
bool httpPost(String path, String jsonPayload);
String httpGet(String path);
String sendAtCommand(String command, const int timeout);


void setup() {
  // Start serial communication with the computer for debugging
  Serial.begin(9600);
  while (!Serial) { }
  Serial.println("Arduino UNO is ready.");

  // Start serial communication with the ESP-01S module
  espSerial.begin(9600); // Common baud rate for ESP-01S
  Serial.println("ESP-01S Serial started at 9600.");

  // TODO: Initialize hardware (motors, sensors, etc.) here
  
  // Setup ESP-01S module (connect to WiFi, etc.)
  setupEsp01s();
}

void loop() {  Serial.println("--- Starting new loop cycle ---");  // 1. Test HTTP POST  Serial.println("Testing HTTP POST...");  JSONVar postPayload;  postPayload["voltage"] = 7.4;  postPayload["status"] = "testing";  String jsonString = JSON.stringify(postPayload);    bool postSuccess = httpPost("/api/status_update", jsonString);  if (postSuccess) {    Serial.println("POST request successful.");  } else {    Serial.println("POST request failed.");  }  delay(2000); // Wait a bit between requests  // 2. Test HTTP GET  Serial.println("\nTesting HTTP GET...");  String responseBody = httpGet("/api/command");  if (responseBody != "") {    Serial.print("GET request successful. Response body: ");    Serial.println(responseBody);  } else {    Serial.println("GET request failed or returned empty body.");  }  // Wait for 10 seconds before the next cycle  Serial.println("\n--- Loop cycle finished. Waiting 10 seconds... ---");  delay(10000);} 

/**
 * @brief Sends a series of AT commands to set up the ESP-01S module.
 */
void setupEsp01s() {
  Serial.println("--- Setting up ESP-01S ---");
  
  // Test AT communication
  sendAtCommand("AT", 2000);
  delay(1000);

  // Reset the module
  sendAtCommand("AT+RST", 2000);
  delay(2000);

  // Set mode to Station mode
  sendAtCommand("AT+CWMODE=1", 2000);
  delay(1000);

  // Connect to WiFi
  String connect_command = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  sendAtCommand(connect_command, 7000); // WiFi connection can take longer
  delay(5000);

  // Check if connected
  String response = sendAtCommand("AT+CIFSR", 2000); // Get IP address
  if (response.indexOf("ERROR") == -1) {
    Serial.println("ESP-01S Connected to WiFi!");
  } else {
    Serial.println("ESP-01S Failed to connect to WiFi.");
  }
  Serial.println("---------------------------");
}

/**
 * @brief Sends a POST request to the server via the ESP-01S.
 * @param path The API path for the request (e.g., "/api/update_status").
 * @param jsonPayload The JSON string to send as the request body.
 * @return true if the request was successful (e.g., received "200 OK"), false otherwise.
 */
bool httpPost(String path, String jsonPayload) {
  String httpRequest = "POST " + path + " HTTP/1.1\r\n";
  httpRequest += "Host: " + String(serverIp) + "\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "Content-Length: " + String(jsonPayload.length()) + "\r\n";
  httpRequest += "Connection: close\r\n\r\n";
  httpRequest += jsonPayload;

  // Establish TCP connection
  String response = sendAtCommand("AT+CIPSTART=\"TCP\",\"" + String(serverIp) + "\",

/**
 * @brief Sends a GET request to the server via the ESP-01S.
 * @param path The API path for the request (e.g., "/api/command").
 * @return A String containing the body of the HTTP response.
 */
String httpGet(String path) {
  String httpRequest = "GET " + path + " HTTP/1.1\r\n";
  httpRequest += "Host: " + String(serverIp) + "\r\n";
  httpRequest += "Connection: close\r\n\r\n";

  // Establish TCP connection
  String response = sendAtCommand("AT+CIPSTART=\"TCP\",\"" + String(serverIp) + "\",\"" + String(serverPort), 5000);
  if (response.indexOf("ERROR") != -1) {
    Serial.println("Failed to establish TCP connection.");
    return "";
  }

  // Send the HTTP request
  sendAtCommand("AT+CIPSEND=" + String(httpRequest.length()), 2000);
  response = sendAtCommand(httpRequest, 5000);

  // Close the connection
  sendAtCommand("AT+CIPCLOSE", 2000);

  // Check for a valid response and extract the body
  int bodyStartIndex = response.indexOf("\r\n\r\n");
  if (response.indexOf("200 OK") != -1 && bodyStartIndex != -1) {
    Serial.println("HTTP GET successful.");
    // Return the response body, which is after the double CRLF
    return response.substring(bodyStartIndex + 4);
  } else {
    Serial.println("HTTP GET failed.");
    return "";
  }
}

/**
 * @brief A utility function to send an AT command and get the response.
 * @param command The AT command to send (without \r\n).
 * @param timeout The maximum time to wait for a response in milliseconds.
 * @return The response from the ESP-01S as a String.
 */
String sendAtCommand(String command, const int timeout) {
  String response = "";
  
  espSerial.println(command); // Send the AT command to the ESP-01S
  
  long int startTime = millis();
  while ((startTime + timeout) > millis()) {
    while (espSerial.available()) {
      char c = espSerial.read();
      response += c;
    }
  }
  
  // Print command and response to the main serial for debugging
  Serial.print("--- AT Command ---\nSent: ");
  Serial.println(command);
  Serial.print("Recv: ");
  Serial.println(response);
  Serial.println("--------------------");
  
  return response;
}

