/*ESPandMPUaccessPoint_V2R1  MJL thepiandi.blogspot.com    August 16, 2017

The purpose of this sketch is to have accelerometers in a handheld
device, change a note played on a Midi device. The idea is that the
handheld device will have no cable connections to any other device.
It will be battery powered so as not to be tethered to anything.

This sketch, along with two other sketchs form a basis that can be 
expanded to provide some interesting musical effects.  

This sketch is for the ESP8266 microcontrollerwhich is wired to a 
MPU9250 nine axix IMU. This device is called the Hand-held Device 

The position of the Hand-held device is derived from the X and Y 
accelerometers. The calculation of position is derived from the raw 
values from the accelerometers and by applying some trigeromerery. 
The position values for both the X and Y axis are from -90.0 to 
90.0 degrees. Those values are reduced to -24 to 24.  Each value of 
the X axis will will represent one note on the scale.
The 49 notes represent 4 octaves with middle C at the zero position.  

The 49 values of the Y position, are used to control the velocity.  A 
value of zero represents a velocity of 64. or in the middle of the range.

A place holder in the data is included for rotation about the Z direction.  
That will be included at a latter time.

The Hand-held device is programmed to be a WiFi access point with its 
own SSID and password. It also has its own URL.

Another ESP8266, called the Base Unit WiFi device, is located is on a base unit.  
The base unit connects to a Midi output device. It connects to the 
Hand-held over WiFi. Once WiFi communications are established, data is 
transmitted between the two devices via TCP/IP sockets.

Sockets allow the connection between the two devices to remain open unless
delibertly closed, and allows for bidirectional communication.  This method
of communications works very well with no perceivable latency.  

The timing of message transmission is controlled by the Base Unit WiFi device.  
Once one device sends data over the Socket, it waits for a response from the 
other.  The Hand-held device makes its measurements and calculations and sends
its data to the Base Unit WiFi device.  That device receives the data and 
processes it.  The Base Unit WiFi devic establishs the measurement rate which 
is approximately 10 measurements per second. This is accomplished by having it
wait 100 ms. after processing its data and before responding back to the 
Hand-held device.  This is accurate enough for the present.  However, if the 
gyroscope positions are to be included, a more accurate method of timing using 
micros() or millis().

The data trasmitted to the Base Unit WiFi devic is in the followint form:
Sxxx,yyy,zzzE followed by CR and LF.  Where xxx and yyy are -24 to 24.
Right now zzz is an arbitary value of -15, and is not used.

It has been noted that the ESP8266 can go into a RESET condition.  After adding
the delay(0) in the loop() function seems to have solved that problem.

The file WebSocketsServer.h is part of the "Links2004 / ArduinoWebSockets" library
by Markus Sattler, not the library "morrissinger / ESP8266-Websocket".  
*/

#include <Wire.h>
#include <My_MPU9250.h>
#include <Arduino.h>
#include <Hash.h>
#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>

/*__________________________________global__________________________________*/
const char *ssid = "Gluon123";
const char *password = "juno5678";

WebSocketsServer webSocket = WebSocketsServer(50009);

int16_t rawAccelX, rawAccelY, rawAccelZ;
long measurementDelay;

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

/*----------------------------------------setupAccels--------------------------*/
void setupAccels(){
  uint8_t AccelFS;
  uint8_t AccelBW;
  

  //**************************************** Set Accel Full Scale
  AccelFS = 0x00;   //Accel full scale equal to +/-2g
  I2CwriteByte(MPU9250_ADDRESS, ACCEL_CONFIG, AccelFS);  
  
  //**************************************** Set Accel BW and Fchoice
  AccelBW = 0x06;   //Accel bandwidth = 5Hz
  I2CwriteByte(MPU9250_ADDRESS, ACCEL_CONFIG2, AccelBW); 
    
}
/*********************************AccelMeasure**************************/
void AccelMeasure(){
  uint8_t fromRegister[6];

  I2Cread(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, fromRegister);

  rawAccelX = (fromRegister[0] <<8) + fromRegister[1];
  rawAccelY = (fromRegister[2] <<8) + fromRegister[3];
  rawAccelZ = (fromRegister[4] <<8) + fromRegister[5]; 
}

/*********************************webSocketEvent************************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  float accelAngleX, accelAngleY, accelAngleZ; 
  float gyroZ = -55.55; 
  int32_t totalVector;
  String toClient;
  int reportToClientX, reportToClientY, reportToClientZ;
  
  if (type == 2){ //WStype = Connect
    webSocket.sendTXT(num, "0");    
   }
    
  if (type == 3){ //WStype = Text
    AccelMeasure();
    totalVector = sqrt((rawAccelX)*(rawAccelX) + (rawAccelY)*(rawAccelY) + (rawAccelZ)*(rawAccelZ));
    
    accelAngleX = asin((float)rawAccelY/totalVector) * ( -57.296);  //57.296 = 2 Pi Radians per 360 deg       
    accelAngleY = asin((float)rawAccelX/totalVector) * 57.296; 
  
    //Make 49 steps from -24 to +24 and convert to int    
    if (accelAngleX >= 0){
      reportToClientX = int(accelAngleX / 3.75 - 0.5);      
    }
    else{
      reportToClientX = int(accelAngleX / 3.75 - 0.5);      
    }

    if (accelAngleY >= 0){
      reportToClientY = int(accelAngleY / 3.75 - 0.5);      
    }
    else{
      reportToClientY= int(accelAngleY / 3.75 - 0.5);      
    }
    
    if (gyroZ >= 0){
      reportToClientZ = int(gyroZ / 3.75 + 0.5);      
    }
    else{
      reportToClientZ = int(gyroZ / 3.75 - 0.5);      
    }

    toClient = "S";
    toClient += String(reportToClientX);
    toClient += ",";
    toClient += String(reportToClientY);
    toClient += ",";
    toClient += String(reportToClientZ);
    toClient += "E\n";      

    webSocket.sendTXT(num, toClient);
  }
}
   
/*__________________________________setup__________________________________*/
void setup() {
  
  //Setup I2C and MPU
  Wire.begin(); 
  setupAccels();

  //Setup Access Point
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  //Setup Socket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

/*__________________________________loop__________________________________*/
void loop() {
  webSocket.loop();
  delay(8);
}
