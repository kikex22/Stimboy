#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// WIFI CREDENTIALS
#define WIFI_SSID "AO9A"
#define WIFI_PASSWORD "1991emgj422      "

// firebase Api key and URL
#define API_KEY "AIzaSyB_HEU7dpE2yj1x8SwoSnoKxNZXkuwYRzg"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://stimboy-1ca74-default-rtdb.firebaseio.com" 


Adafruit_MPU6050 mpu;
//C0:49:EF:6C:07:24

uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0x6C, 0x07, 0x24}; 

typedef struct struct_message {
  
  
  float hi;
  float mpu;
  float ytest;
  
  
} struct_message;

// el nombre del struct es my Data
struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  
  
}





void setup() {
  
  WiFi.mode(WIFI_STA);
 if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  else
  {
    Serial.println("Succes: Added peer");
  } 
  Serial.begin(115200);  // Iniciar puerto serial
  while (!Serial) {
    delay(10);
  }
  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);  // Ajustar rango del acelerómetro a 2G
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);       // Ajustar rango del giroscopio a 250 grados por segundo

  Serial.println("Sensor iniciado correctamente");

}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accel_ang_x = atan2(a.acceleration.x, sqrt(pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2))) * 180 / 3.14;
  float accel_ang_y = atan2(a.acceleration.y, sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.z, 2))) * 180 / 3.14;

 // Serial.print("Inclinación en X: ");
  //Serial.print(accel_ang_x);
 // Serial.print("\tInclinación en Y: ");
  //Serial.println(accel_ang_y);


 esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }


 myData.mpu=accel_ang_x; 
 myData.ytest= accel_ang_y;

  
}