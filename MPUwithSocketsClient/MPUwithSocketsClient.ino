/*MPUwithSocketsClient  MJL thepiandi.blogspot.com    July 19, 2017

This is a proof of concept sending messages between two ESP82666 MCUs.

This sketch is for the client ESP8266 which  is wired to three LEDs: 
RED, GREEN, and YELLOW.  The server ESP8266 is wired to a MPU9250 nine 
axix IMU.

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

The file WebSocketsClient.h is part of the "Links2004 / ArduinoWebSockets" library
by Markus Sattler, not the library "morrissinger / ESP8266-Websocket". 
 
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Hash.h>

/*__________________________________global__________________________________*/
WebSocketsClient webSocket;

const char *ssid = "Gluon123";
const char *password = "juno5678";

const char* host = "192.168.4.1"; 
const int httpPort = 50009; 

#define RED 12
#define YELLOW 13
#define GREEN 14

//#define DEBUG

/*********************************webSocketEvent***********************/
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  
  #if defined(DEBUG)   
    Serial.print("type: ");
    Serial.print(type);
    Serial.print("\t\t");
    for (int i = 0; i < length; i++){
      Serial.print(int(payload[i]) - 48);    
    }
    Serial.println("");   
  #endif

  
  if (type == 2){ //If Server Just Connected - All LEDs Off
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, HIGH);
    digitalWrite(YELLOW, HIGH);    
  }  
  if (type == 3){
    switch (int(payload[0]) - 48){
      case 0:  //All LEDs Off
        digitalWrite(RED, HIGH); //Turn Off RED LED
        digitalWrite(GREEN, HIGH); //Turn Off GREEN LED
        digitalWrite(YELLOW, HIGH); //Turn Off YELLOW LED
        break;
      
      case 1:  //X Gyro turned in positive direction
        digitalWrite(RED, LOW); //Turn On RED LED
        break;

      case 2:  //Y Gyro turned in positive direction
        digitalWrite(GREEN, LOW); //Turn On GREEN LED
        break;

      case 3:  //Z Gyro turned in positive direction
        digitalWrite(YELLOW, LOW); //Turn On YELLOW LED
        break;
        
      case 4:  //X Gyro turned in negative direction
        digitalWrite(RED, HIGH); //Turn On RED LED
        break;

      case 5:  //Y Gyro turned in negative direction
        digitalWrite(GREEN, HIGH); //Turn On GREEN LED
        break;

      case 6:  //Z Gyro turned in negative direction
        digitalWrite(YELLOW, HIGH); //Turn On YELLOW LED
        break;
    }
  }

  delay(25);
  //delay(2000);
  webSocket.sendTXT("OK");
}

//__________________________________setup__________________________________
void setup() {
  #if defined(DEBUG)   
    Serial.begin(115200);
  #endif
  
  //Setting up LEDs
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);

  //turn all LEDs Off
  digitalWrite(RED, HIGH);
  digitalWrite(YELLOW, HIGH);
  digitalWrite(GREEN, HIGH);
  
  // Connecting to Access Point
  #if defined(DEBUG)   
    Serial.print("\n\nConnecting to ");
    Serial.println(ssid);
  #endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if defined(DEBUG)   
      Serial.print(".");
    #endif
  }

  #if defined(DEBUG)   
    Serial.println("\nWiFi connected\n");  
  #endif

  //flash LEDs
  for (int i = 0; i < 5; i++){
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, LOW);
    delay(500);
    digitalWrite(RED, HIGH);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(GREEN, HIGH);
    delay(500);
  }

  delay(2000);

  //Setup Websocket
  webSocket.begin(host, httpPort);
  webSocket.onEvent(webSocketEvent);

}

void loop() {
    webSocket.loop();
    delay(0);
}
