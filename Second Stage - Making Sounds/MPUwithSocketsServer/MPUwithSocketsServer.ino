/*MPUwithSocketsServer  MJL thepiandi.blogspot.com    July 19, 2017

This is a proof of concept sending messages between two ESP82666 MCUs.

This sketch is for the server ESP8266 which is wired to a MPU9250 nine axix
IMU.  The client ESP8266 is wired to three LEDs: RED, GREEN, and YELLOW.

Measures rotation from the MPU9250's gyroscope.  If any of the three axis
rotation measure over 100 dps, it is determined which axis rotation is the 
greatest and the direction.  That information is sent to the client ESP8266
as a number between 1 and 6 (see below for details).

The method of transmission between server and client is via TCP/IP Sockets.

Sockets allow the connection between server and client to remain open unless
delibertly closed, and allows for bidirectional communication.  

In this instance, the server sends, as payload, the number between 1 and 6,
unless the client has just connected, in which case, a 0 is sent (to turn off
all LEDs).  The client responds be sending back the String "OK" after a small
delay.  This delay represents the time between measurements dicatated by the 
bandwidth selected.  The client has control over the LEDs.

This method oc communications works very well with no perceivable latency.  While
the message sent from the server is simply one character, it certainly is 
possible to send much longer strings.

It has been noted that the ESP8266 can go into a RESET condition.  After adding
the delay(0) in the loop() function seems to have solved that problem.

The file WebSocketsServer.h is part of the "Links2004 / ArduinoWebSockets" library
by Markus Sattler, not the library "morrissinger / ESP8266-Websocket".  
*/
 
#include <Arduino.h>
#include <Hash.h>
#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <My_MPU9250.h>

/*__________________________________global__________________________________*/
const char *ssid = "Gluon123";
const char *password = "juno5678";

WebSocketsServer webSocket = WebSocketsServer(50009);

//#define DEBUG

/*-------------------------------------I2Cread--------------------------------*/
void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t* Data){
  uint8_t index=0;
  
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();
  
  // Read Nbytes
  Wire.requestFrom(Address, Nbytes); 
  while (Wire.available())
    Data[index++] = Wire.read();
}

/*-------------------------------------I2CwriteByte------------------------------*/
void I2CwriteByte(uint8_t Address, uint8_t Register, uint8_t Data){
  // Only one byte is written
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}

/*----------------------------------------setupGyros--------------------------*/
void setupGyros(){
  uint8_t gyroConfig;
  uint8_t gyroBW;
  
  //**************************************** Disable Accelerometer
  //disable accelerometer
  I2Cread(MPU9250_ADDRESS, PWR_MGMT_2, 1, &gyroConfig);
  gyroConfig &= 0x38;
  //Now write it out
  I2CwriteByte(MPU9250_ADDRESS, PWR_MGMT_2, gyroConfig);  

  //**************************************** Set Full Scale
  //read present gyro configuration
  I2Cread(MPU9250_ADDRESS, GYRO_CONFIG, 1, &gyroConfig);
  //for gyro full scale equal to +/-250dps
  gyroConfig = gyroConfig & 0b11100111;
  //for gyro full scale equal to +/-500dps
  //gyroConfig = gyroConfig & 0b11100111 | 0b00001000;
  //for gyro full scale equal to +/-1000dps
  //gyroConfig = gyroConfig & 0b11100111 | 0b00010000;
  //for gyro full scale equal to +/-2000dps
  //gyroConfig = gyroConfig | 0b00011000;
  
  //**************************************** Set BW
  //read present configuration
  I2Cread(MPU9250_ADDRESS, CONFIG, 1, &gyroConfig);
  gyroConfig &= 0xF9; //For 184Hz Bandwidth
  I2CwriteByte(MPU9250_ADDRESS, CONFIG, gyroConfig); 
  //set FCHOICE_B according to BW selected 
  I2Cread(MPU9250_ADDRESS, GYRO_CONFIG, 1, &gyroConfig);
  gyroConfig &= 0xFC;
  I2CwriteByte(MPU9250_ADDRESS, GYRO_CONFIG, gyroConfig);     
}

/*********************************gyroMeasure**************************/
int gyroMeasure(){
  float range = 250.0;
  int16_t rawMeasurement;
  uint8_t fromRegister[2];

  float gyroX, gyroY, gyroZ;
  static int maxGyro;

  //read gyro X output from MPU9250
  I2Cread(MPU9250_ADDRESS, GYRO_XOUT_H, 2, fromRegister);
  rawMeasurement = (fromRegister[0] << 8) + fromRegister[1];
  gyroX = range * float(rawMeasurement) / 32768.0; 
  
  //read gyro Y output from MPU9250
  I2Cread(MPU9250_ADDRESS, GYRO_YOUT_H, 2, fromRegister);
  rawMeasurement = (fromRegister[0] << 8) + fromRegister[1];
  gyroY = range * float(rawMeasurement) / 32768.0; 
  
  //read gyro Z output from MPU9250
  I2Cread(MPU9250_ADDRESS, GYRO_ZOUT_H, 2, fromRegister);
  rawMeasurement = (fromRegister[0] << 8) + fromRegister[1];
  gyroZ = range * float(rawMeasurement) / 32768.0;
  
  if ((abs(gyroX) >= 100.0) | (abs(gyroY) >= 100.0) | (abs(gyroZ) >= 100.0)){
    if (abs(gyroX) >= abs(gyroY)){
      maxGyro = 1;
      if (gyroX < 0){
        maxGyro = 4;
      }
      if (abs(gyroZ) >= abs(gyroX)){
        maxGyro = 3;
        if (gyroZ < 0){
          maxGyro = 6;
        }
      }
    }
    else {
      maxGyro = 2;
      if (gyroY < 0){
        maxGyro = 5;
      }
      if (abs(gyroZ) >= abs(gyroY)){
        maxGyro = 3;
        if (gyroZ < 0){
          maxGyro = 6;
        }
      }    
    }
  }
  
  #if defined(DEBUG)   
    Serial.print("gyroX: ");
    Serial.println(gyroX);
    Serial.print("gyroY: ");
    Serial.println(gyroY);
    Serial.print("gyroZ: ");
    Serial.println(gyroZ);

    Serial.print("MaxGyro: ");
    Serial.println(maxGyro);    
  #endif
  
  return (maxGyro);  
}
/*********************************webSocketEvent************************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  String fromGyro;
  
  if (type == 2){ //WStype = Connect
    //turn All REDs LED Off
    webSocket.sendTXT(num, "0");    
    #if defined(DEBUG)   
      Serial.println("Turn All LEDs OFF");
    #endif
   }
    
  if (type == 3){ //WStype = Text
    fromGyro = String(gyroMeasure());
    webSocket.sendTXT(num, fromGyro);
    #if defined(DEBUG)   
      Serial.print("Sending To Client: ");
      Serial.println(fromGyro);
    #endif
    
  } 
}


/*__________________________________setup__________________________________*/
void setup() {
  #if defined(DEBUG)   
    Serial.begin(115200);
  #endif

  //Setup I2C and MPU
  Wire.begin();
  setupGyros();
  
  //Setup Access Point
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);
  #if defined(DEBUG)   
    Serial.println("\n\nConfiguring Access Point and Starting Server");
  #endif

  IPAddress myIP = WiFi.softAPIP();

  #if defined(DEBUG)   
    Serial.print("Access Point IP Address: ");
    Serial.println(myIP);
  #endif
  
  //Setup Socket
  webSocket.begin();
  #if defined(DEBUG)   
    Serial.println("Server Started");
  #endif

  webSocket.onEvent(webSocketEvent);
}

/*__________________________________loop__________________________________*/
void loop() {
  webSocket.loop();
  delay(8);
}
