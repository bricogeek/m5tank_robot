#include <m5unified.h>
#include <esp_now.h>
#include <WiFi.h>
#include <M5GFX.h>
#include <Adafruit_VL6180X.h>
#include <math.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

// ATOM S3 Tank Mac Address: DC:54:75:D0:9A:08
const int MOTOR_LEFT = 5;
const int MOTOR_RIGHT = 6;

const int MLEFT = 0;
const int MRIGHT = 1;

// setting PWM properties
const int pwmfreq1 = 400;
const int pwmResolution1 = 8;

const int pwmfreq2 = 400;
const int pwmResolution2 = 8;

int mode = 0; // 0: tank 1: distancia

Adafruit_VL6180X vl = Adafruit_VL6180X();

// Structure example to receive data
// Must match the sender structure
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

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rfData, incomingData, sizeof(rfData));
}

//==========================================================================================
void setup() {

    M5.begin();
    M5.Power.begin();      
    Serial.begin (115200);      

    // espera al dispositivo USB
    while (!Serial) {
      delay(1);
    }    

    Serial.println("Serial OK");          

    Wire.begin(38, 39);

    // Borramos la pantalla
    M5.Lcd.fillScreen(TFT_BLACK);

    // Activamos Wifi
    WiFi.mode(WIFI_STA);

    // Iniciamos ESP-NOW
    if (esp_now_init() != ESP_OK) {
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(5, 5);
      M5.Lcd.setTextColor(TFT_RED);
      M5.Lcd.println("ESP-NOW Error");
      return;
    }
    
    // Función que recibe los datos ESP-NOW desde el emisor    
    esp_now_register_recv_cb(OnDataRecv);    

    // M5Tank!
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.println("M5Tank");

    // Mostramos la dirección MAC    
    //M5.Lcd.setTextSize(1);
    //M5.Lcd.setCursor(5, 50);
    //M5.Lcd.setTextColor(TFT_BLUE);
    //M5.Lcd.println(WiFi.macAddress());

    // Datos IMU recibidos
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 50);      
    M5.Lcd.println("X: 0.0");    
    M5.Lcd.setCursor(5, 68);      
    M5.Lcd.println("Y: 0.0");    
    M5.Lcd.setCursor(5, 86);      
    M5.Lcd.println("Z: 0.0");            

    // Inicializa el sensor laser
    if (!vl.begin()) {
      M5.Lcd.setTextColor(TFT_RED);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(5, 30);      
      M5.Lcd.println("VL6180 error!");      
    }    
    else
    {
      M5.Lcd.setTextColor(TFT_GREEN);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setCursor(5, 30);    
      M5.Lcd.println("VL6180 OK");    
    }
    
    // configure LED PWM functionalitites
    ledcSetup(MLEFT, pwmfreq1, pwmResolution1);
    ledcSetup(MRIGHT, pwmfreq2, pwmResolution2);
    
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(MOTOR_LEFT, MLEFT);    
    ledcAttachPin(MOTOR_RIGHT, MRIGHT);    

}

// MLEFT or MRIGHT
// Speed: 0-255
void setMotorSpeed(int motor, int speed) { ledcWrite(motor, speed); }

//==========================================================================================
void loop() {
/*
M5.Lcd.setCursor(5,5);
M5.Lcd.print("Hello: ");  

M5.Lcd.setCursor(20,5);
M5.Lcd.print(random(0, 10000));  
*/


    M5.update();

    M5.Lcd.fillRect(0, 50, 128, 50, TFT_BLUE);
 
    // Datos IMU recibidos
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 50);      
    M5.Lcd.print("LEFT: ");    
    M5.Lcd.println(rfData.left);    

    M5.Lcd.setCursor(5, 68);      
    M5.Lcd.print("RIGHT: ");    
    M5.Lcd.println(rfData.right);

    M5.Lcd.setCursor(5, 86);      
    M5.Lcd.print("RUN: ");            
    M5.Lcd.println(rfData.run);

    // Cambiamos modo
    if (M5.BtnA.wasClicked())
    {
      if (mode == 0) { mode = 1; }
      else { mode = 0; }    
    }    

    M5.Lcd.fillRect(0, 100, 128, 28, TFT_BLACK);    
  
    if (mode == 0)
    {
      if (rfData.run == 1)
      {    
        
        setMotorSpeed(MLEFT, 50);
        setMotorSpeed(MRIGHT, 200);      

      }
      else if (rfData.left == 1)
      {
        setMotorSpeed(MLEFT, 50);
        setMotorSpeed(MRIGHT, 50);
      }
      else if (rfData.right == 1)
      {
        setMotorSpeed(MLEFT, 200);
        setMotorSpeed(MRIGHT, 200);
      }  
      else {
        setMotorSpeed(MLEFT, 0);
        setMotorSpeed(MRIGHT, 0);    
      }
    }
  else { // Mode 1
      // Leemos distancia
      uint8_t range = vl.readRange();
      uint8_t status = vl.readRangeStatus();  

      int distancia = 0;
      if (status == VL6180X_ERROR_NONE) {
        distancia = range;
      }  

      // Barra de distancia
      M5.Lcd.fillRect(0, 110, distancia, 18, TFT_RED);

      if (distancia > 0)
      {
        if (distancia > 90)
        {
          setMotorSpeed(MLEFT, 50);
          setMotorSpeed(MRIGHT, 200);      
        }
        else
        {
          if (distancia < 110)
          {
            setMotorSpeed(MLEFT, 200);
            setMotorSpeed(MRIGHT, 50);      
          }
        }        
      }
      else
      {
        setMotorSpeed(MLEFT, 0);
        setMotorSpeed(MRIGHT, 0);            
      }

    }

    if (mode == 0) { M5.Lcd.fillCircle(120, 10, 8, TFT_GREEN); }
    else { M5.Lcd.fillCircle(120, 10, 8, TFT_RED); }    

    delay(10);    

}
//==========================================================================================
