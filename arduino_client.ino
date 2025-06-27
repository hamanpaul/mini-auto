

/*
  Arduino API Client for Vehicle Control

  This sketch demonstrates how to connect to a WiFi network and interact with a 
  Python FastAPI server to send vehicle status updates and receive movement commands.

  HARDWARE:
  - ESP8266 or ESP32 based board

  LIBRARIES:
  - ESP8266WiFi (for ESP8266) or WiFi (for ESP32)
  - Arduino_JSON by Arduino
  - ESP8266HTTPClient (for ESP8266) or HTTPClient (for ESP32)

  SETUP:
  1. Install the required libraries via the Arduino IDE's Library Manager.
  2. Update the `ssid`, `password`, and `serverIp` variables below.
  3. Upload the sketch to your ESP board.
  4. Open the Serial Monitor at 115200 baud to see the output.
*/

// Include necessary libraries based on your board
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <Arduino_JSON.h>

// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";         // Your WiFi network SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi network password
const char* serverIp = "192.168.1.100";      // The IP address of your computer running the Python server
const int serverPort = 8000;

// --- Function Declarations ---
void connectToWiFi();
bool sendStatusUpdate(int batteryLevel, const char* currentState);
String getMovementCommand(const char* command);

void setup() {
  Serial.begin(115200);
  while (!Serial) { } // Wait for serial connection

  connectToWiFi();
}

void loop() {
  // --- Example Usage ---

  // 1. Send a status update to the server
  Serial.println("\n--- Sending Status Update ---");
  bool success = sendStatusUpdate(98, "stopped");
  if (success) {
    Serial.println("Status update sent successfully.");
  } else {
    Serial.println("Failed to send status update.");
  }

  delay(5000); // Wait 5 seconds

  // 2. Request a movement command from the server
  Serial.println("\n--- Requesting Movement Command ---");
  String commandAction = getMovementCommand("forward");
  if (commandAction != "") {
    Serial.print("Received command: ");
    Serial.println(commandAction);
    // Here you would add your motor control logic based on the command
    // e.g., if (commandAction == "前進") { moveMotorsForward(); }
  } else {
    Serial.println("Failed to get movement command or invalid response.");
  }
  
  delay(10000); // Wait 10 seconds before the next cycle
}

/**
 * @brief Connects the ESP board to the configured WiFi network.
 */
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Sends a vehicle status update to the server via a POST request.
 * @param batteryLevel The current battery percentage.
 * @param currentState A string describing the vehicle's current state (e.g., "stopped").
 * @return true if the request was successful (HTTP 200), false otherwise.
 */
bool sendStatusUpdate(int batteryLevel, const char* currentState) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return false;
  }

  WiFiClient client;
  HTTPClient http;
  
  String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + "/status";
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  JSONVar payload;
  payload["battery"] = batteryLevel;
  payload["current_state"] = currentState;
  String jsonPayload = JSON.stringify(payload);

  Serial.print("Sending POST to: ");
  Serial.println(serverUrl);
  Serial.print("Payload: ");
  Serial.println(jsonPayload);

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String responseBody = http.getString();
    Serial.print("Response body: ");
    Serial.println(responseBody);
    http.end();
    return (httpResponseCode == 200);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

/**
 * @brief Gets a movement command from the server via a GET request.
 * @param command The command to request (e.g., "forward", "backward").
 * @return A String containing the "action" from the JSON response, or an empty string on failure.
 */
String getMovementCommand(const char* command) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return "";
  }

  WiFiClient client;
  HTTPClient http;
  
  String serverUrl = "http://" + String(serverIp) + ":" + String(serverPort) + "/" + String(command);
  http.begin(client, serverUrl);

  Serial.print("Sending GET to: ");
  Serial.println(serverUrl);

  int httpResponseCode = http.GET();
  String action = "";

  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.print("Response payload: ");
    Serial.println(payload);

    // Parse the JSON response
    JSONVar myObject = JSON.parse(payload);

    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
    } else if (myObject.hasOwnProperty("action")) {
      // Extract the "action" value
      action = (const char*) myObject["action"];
    }
  } else {
    Serial.print("Error on sending GET: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return action;
}

