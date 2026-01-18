#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include <MPU6050.h>
#include <I2Cdev.h>
#include <Wire.h>

// WiFi Credentials
const char* ssid = "Saad";
const char* password = "esp32saad";

// Create AsyncWebServer and WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Motor Pins
#define leftMotorPWMPin 6
#define leftMotorDirPin 7
#define rightMotorPWMPin 5
#define rightMotorDirPin 16

// PID Parameters (default values)
float Kp = 20.0;
float Kd = 0.05;
float Ki = 0.0;
float targetAngle = -2.5;

const float sampleTime = 0.005;

// MPU6050
MPU6050 mpu;

// Variables
int16_t accX, accZ, gyroY;
volatile int motorPower;
volatile float gyroRate;
volatile float accAngle, gyroAngle, currentAngle, prevAngle = 0;
volatile float error, prevError = 0, errorSum = 0;
unsigned long prevTime = 0;

// Slider values as strings for JSON
String sliderKp = "20";
String sliderKi = "0";
String sliderKd = "0";
String sliderTarget = "-2.5";

// JSON Variable
JSONVar sliderValues;

// Get Slider Values
String getSliderValues() {
  sliderValues["Kp"] = sliderKp;
  sliderValues["Ki"] = sliderKi;
  sliderValues["Kd"] = sliderKd;
  sliderValues["target"] = sliderTarget;
  
  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Set Motors
void setMotors(int leftMotorSpeed, int rightMotorSpeed) {
  if (leftMotorSpeed >= 0) {
    analogWrite(leftMotorPWMPin, leftMotorSpeed);
    digitalWrite(leftMotorDirPin, LOW);
  } else {
    analogWrite(leftMotorPWMPin, 255 + leftMotorSpeed);
    digitalWrite(leftMotorDirPin, HIGH);
  }
  
  if (rightMotorSpeed >= 0) {
    analogWrite(rightMotorPWMPin, rightMotorSpeed);
    digitalWrite(rightMotorDirPin, LOW);
  } else {
    analogWrite(rightMotorPWMPin, 255 + rightMotorSpeed);
    digitalWrite(rightMotorDirPin, HIGH);
  }
}

// Notify all WebSocket clients
void notifyClients(String data) {
  ws.textAll(data);
}

// Handle WebSocket Messages
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    
    // Handle Kp slider
    if (message.indexOf("Kp") >= 0) {
      sliderKp = message.substring(2);
      Kp = sliderKp.toFloat();
      Serial.print("Kp updated: ");
      Serial.println(Kp);
      notifyClients(getSliderValues());
    }
    // Handle Ki slider
    else if (message.indexOf("Ki") >= 0) {
      sliderKi = message.substring(2);
      Ki = sliderKi.toFloat();
      Serial.print("Ki updated: ");
      Serial.println(Ki);
      notifyClients(getSliderValues());
    }
    // Handle Kd slider
    else if (message.indexOf("Kd") >= 0) {
      sliderKd = message.substring(2);
      Kd = sliderKd.toFloat();
      Serial.print("Kd updated: ");
      Serial.println(Kd);
      notifyClients(getSliderValues());
    }
    // Handle target angle slider
    else if (message.indexOf("target") >= 0) {
      sliderTarget = message.substring(6);
      targetAngle = sliderTarget.toFloat();
      Serial.print("Target Angle updated: ");
      Serial.println(targetAngle);
      notifyClients(getSliderValues());
    }
    // Get current values
    else if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getSliderValues());
    }
  }
}

// WebSocket Event Handler
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, 
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", 
                    client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// Initialize WebSocket
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);}
    Serial.println("Serial started");
  
  // Motor pins
  pinMode(leftMotorPWMPin, OUTPUT);
  pinMode(leftMotorDirPin, OUTPUT);
  pinMode(rightMotorPWMPin, OUTPUT);
  pinMode(rightMotorDirPin, OUTPUT);
  pinMode(13, OUTPUT);
  
  // Initialize I2C with ESP32-S3 safe pins
  Wire.begin(8, 9);  // SDA=GPIO8, SCL=GPIO9
  
  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 NOT FOUND");
    while (1);
  }
  Serial.println("MPU6050 Found");
  
  // Set MPU6050 offsets
  mpu.setXAccelOffset(-1853);
  mpu.setYAccelOffset(261);
  mpu.setZAccelOffset(4506);
  mpu.setXGyroOffset(-64);
  mpu.setYGyroOffset(15);
  mpu.setZGyroOffset(52);
  
  // Initialize filesystem and WiFi
  initFS();
  initWiFi();
  initWebSocket();
  
  // Web Server Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", LittleFS, "/");
  
  // Start server
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  unsigned long currTime = micros();
  
  // Run PID control at 200Hz (5000 microseconds = 5ms)
  if (currTime - prevTime >= 5000) {
    prevTime = currTime;
    
    // Read MPU6050
    accX = mpu.getAccelerationX();
    accZ = mpu.getAccelerationZ();
    gyroY = mpu.getRotationY();
    
    // Calculate angles
    accAngle = atan2(accX, accZ) * RAD_TO_DEG;
    gyroRate = gyroY / 131.0;
    currentAngle = 0.9934 * (prevAngle + gyroRate * sampleTime) + 0.0066 * accAngle;
    
    // PID calculation
    error = currentAngle - targetAngle;
    errorSum += error;
    errorSum = constrain(errorSum, -300, 300);
    
    motorPower = Kp * error + Ki * errorSum * sampleTime - Kd * (currentAngle - prevAngle) / sampleTime;
    motorPower = constrain(motorPower, -255, 255);
    
    // Set motors
    setMotors(motorPower, motorPower);
    
    prevAngle = currentAngle;
  }
  
  // Clean up WebSocket clients
  ws.cleanupClients();
}