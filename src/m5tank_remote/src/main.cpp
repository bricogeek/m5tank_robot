#include <Arduino.h>
#include <M5Stack.h>
#include <esp_now.h>
#include <WiFi.h>

// Dirección MAC del tanque
uint8_t tankMAC[] = { 0xDC, 0x54, 0x75, 0xD0, 0x9A, 0x08 };

typedef struct struct_message {
    bool left;
    bool run;
    bool right;
    float x;
    float y;
    float z;
} struct_message;

// Create a struct_message called myData
struct_message rfData;

// Create peer interface
esp_now_peer_info_t peerInfo;

float pitch, roll, yaw;
float x, y, z;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Power.begin();

  M5.Imu.Init();
  M5.Imu.getAhrsData(&pitch, &roll, &yaw);

  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, tankMAC, sizeof(tankMAC));
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }  

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(55, 0);
  M5.Lcd.println("M5Tank Remote");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(35, 25);
  M5.Lcd.println("Tanks everwhere! ;)");  

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Lcd.setCursor(0, 200);
  M5.Lcd.println("   LEFT     RUN    RIGHT");        
  
}

void loop() {
  // put your main code here, to run repeatedly:

  //M5.Imu.getAhrsData(&pitch, &roll, &yaw);
  M5.Imu.getAccelData(&x, &y, &z);

  rfData.left = false;
  rfData.right = false;
  rfData.run = false;

  rfData.x = 0;
  rfData.y = 0;
  rfData.z = 0;

  if (M5.BtnA.isPressed())
  {    
    rfData.left = true;
    M5.Lcd.setTextSize(5);
    M5.Lcd.setCursor(100, 100);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.println("LEFT");      
  }
  else if (M5.BtnB.isPressed())
  {    
    rfData.run = true;
    M5.Lcd.setTextSize(5);
    M5.Lcd.setCursor(100, 100);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.println("RUN");      
  }
  else if (M5.BtnC.isPressed())
  {   
    rfData.right = true;
    M5.Lcd.setTextSize(5);
    M5.Lcd.setCursor(100, 100);
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.println("RIGHT");      
  }
  else
  {
    M5.Lcd.fillRect(0, 50, 320, 130, TFT_BLUE);
  }

  // Datos IMU
  M5.Lcd.fillRect(0, 50, 0, 10, TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_PINK);
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.println(x);  

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(tankMAC, (uint8_t *) &rfData, sizeof(rfData));
   
  if (result == ESP_OK) {
    //Serial.println("Sent with success");
  }
  else {
    //Serial.println("Error sending the data");
  }  

  M5.update();

  delay(10);

}