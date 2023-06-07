#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_camera.h"
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Replace with your MQTT broker address and port
const char* mqtt_server = "your_MQTT_broker_address";
const int mqtt_port = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

camera_config_t config;
camera_fb_t* fb;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("WiFi connected");

  // Setup camera
  config.ledc_timer = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Connect to MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP32CAM")) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT failed with state ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void loop() {
  // Capture image
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Convert image to PNG format
  size_t pngSize = 0;
  uint8_t* pngData = NULL;
  bool ok = frame2png(fb, &pngData, &pngSize);
  if (!ok) {
    Serial.println("Failed to convert image to PNG");
    esp_camera_fb_return(fb);
    return;
  }

  // Create JSON message
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["type"] = "image";
  jsonDoc["data"] = base64::encode(pngData, pngSize);
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  
  // Publish message to MQTT broker
  if (mqttClient.publish("image", jsonString.c_str())) {
    Serial.println("Image sent");
  } else {
    Serial
