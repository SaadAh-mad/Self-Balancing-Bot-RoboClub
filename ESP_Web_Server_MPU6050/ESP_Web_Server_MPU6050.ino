#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// ===== WiFi Credentials =====
const char* ssid = "Saad";
const char* password = "esp32saad";

// ===== Web Server =====
AsyncWebServer server(80);
AsyncEventSource events("/events");

// ===== MPU Variables =====
float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float tiltAcc = 0.0; 

// ===== Timer =====
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ===== I2C (ESP32-S3 SAFE PINS) =====
  Wire.begin(8, 9);   // SDA=GPIO8, SCL=GPIO9 (change if needed)

  Serial.println("Initializing MPU6050...");
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("❌ MPU6050 NOT FOUND");
    while (1);
  }
  Serial.println("✅ MPU6050 Found");

  // ===== LittleFS =====
  if (!LittleFS.begin(true)) {
    Serial.println("❌ LittleFS Mount Failed");
    return;
  }
  Serial.println("✅ LittleFS Mounted");

  // ===== WiFi =====
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("✅ IP Address: ");
  Serial.println(WiFi.localIP());

  // ===== ROUTES =====

  // Serve index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  // Serve static files
  server.serveStatic("/", LittleFS, "/");

  // RESET buttons
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OK");
  });
  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroX = 0;
    request->send(200, "text/plain", "OK");
  });
  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroY = 0;
    request->send(200, "text/plain", "OK");
  });
  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroZ = 0;
    request->send(200, "text/plain", "OK");
  });

  // ===== EVENTS =====
  events.onConnect([](AsyncEventSourceClient *client) {
    Serial.println("✅ SSE Client Connected");
  });

  server.addHandler(&events);

  // ===== START SERVER =====
  server.begin();
  Serial.println("🚀 Web server started");
}

void loop() {
  if (millis() - lastTime > 100) {
    lastTime = millis();

    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Convert values
    accX = ax / 16384.0;
    accY = ay / 16384.0;
    accZ = az / 16384.0;

    tiltAcc = atan2(accY, accZ);  // in radians
    float tiltDeg = tiltAcc * 180.0 / PI;  //converting thsi to degree

    gyroX = gx / 131.0 * DEG_TO_RAD;
    gyroY = gy / 131.0 * DEG_TO_RAD;
    gyroZ = gz / 131.0 * DEG_TO_RAD;

    Serial.print("ACC (g): ");
Serial.print(accX, 3); Serial.print(", ");
Serial.print(accY, 3); Serial.print(", ");
Serial.print(accZ, 3);

Serial.print(" | GYRO (rad/s): ");
Serial.print(gyroX, 3); Serial.print(", ");
Serial.print(gyroY, 3); Serial.print(", ");
Serial.println(gyroZ, 3);

    // ===== SEND GYRO =====
    String gyroData = "{";
    gyroData += "\"gyroX\":" + String(gyroX, 4) + ",";
    gyroData += "\"gyroY\":" + String(gyroY, 4) + ",";
    gyroData += "\"gyroZ\":" + String(gyroZ, 4);
    gyroData += "}";

    events.send(gyroData.c_str(), "gyro_readings", millis());

    // ===== SEND ACC =====
    String accData = "{";
    accData += "\"accX\":" + String(accX, 4) + ",";
    accData += "\"accY\":" + String(accY, 4) + ",";
    accData += "\"accZ\":" + String(accZ, 4);
    accData += "}";

    events.send(accData.c_str(), "accelerometer_readings", millis());

    String tiltData = "{";
    tiltData += "\"tilt\":" + String(tiltDeg, 2);
    tiltData += "}";
    events.send(tiltData.c_str(), "tilt", millis());

  }
}
