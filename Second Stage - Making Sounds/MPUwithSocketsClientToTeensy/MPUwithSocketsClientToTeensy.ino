/*MPUwithSocketsClientToTeensy  MJL thepiandi.blogspot.com    August 1, 2017

This is a progression of my sketch "MPUwithSocketsClient" of July 19,2017

In this iteration, the three LEDs, RED, GREEN, and YELLOW, are removed from
the circuit. A connection to a Teensy miorocontroller has been added: This
ESP Serial1 (pin TXD1)to Teensy Serial1 (pin RX1).

Teensy is added because it has a true USB - MIDI interface which should be 
recognized by the Room of Sound Midi Distributor.  

The ESP8266 Server hardware and sketch are not changed for this iteration.

The Server Measures rotation from the MPU9250's gyroscope.  If any of the three axis
rotation measure over 100 dps, it is determined which axis rotation is the 
greatest and the direction.  That information is sent to the client ESP8266
as a number between 1 and 6 (see below for details).

The method of transmission between server and client is via TCP/IP Sockets.
The method of transmission between client and Teensy is via Serial.

Sockets allow the connection between server and client to remain open unless
delibertly closed, and allows for bidirectional communication.  

In this instance, the server sends, as payload, the number between 1 and 6,
unless the client has just connected, in which case, a 0 is sent (to turn off
all LEDs).  The client responds be sending back the String "OK" after a small
delay.  This delay represents the time between measurements dicatated by the 
bandwidth selected.  

This method oF communications works very well with no perceivable latency.  While
the message sent from the server is simply one character, it certainly is 
possible to send much longer strings.  The same single character (followed by CR and
LF) are sent to the Teensy.

It has been noted that the ESP8266 can go into a RESET condition.  After adding
the delay(0) in the loop() function seems to have solved that problem.

The blue LED on the ESP flashes five times when a connection is made beween the
server and clinet ESP.

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

#define LED 16


//#define DEBUG

/*********************************webSocketEvent***********************/
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  static int oldGyro;
  int newGyro = int(payload[0]);
  
  #if defined(DEBUG)   
    Serial.print("type: ");
    Serial.print(type);
    Serial.print("\t\t");
    for (int i = 0; i < length; i++){
      Serial.print(int(payload[i]) - 48);    
    }
    Serial.println("");   
  #endif

  
  if (type == 3){
    if (newGyro != oldGyro){
      oldGyro = newGyro;
      //Send to Teensy
      Serial1.println(newGyro - 48);
    }
  }
  
  delay(25);
  //delay(2000);
  webSocket.sendTXT("OK");
}

/*__________________________________setup__________________________________*/
void setup() {
  #if defined(DEBUG)   
    Serial.begin(115200);
  #endif
  //Serial Connection to Teensy
  Serial1.begin(115200);
  
  //Setting up LED
  pinMode(LED, OUTPUT);

  //turn LED Off
  digitalWrite(LED, HIGH);
  
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

  //flash LED
  for (int i = 0; i < 5; i++){
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
  }

  delay(2000);

  //Setup Websocket
  webSocket.begin(host, httpPort);
  webSocket.onEvent(webSocketEvent);

}
/*__________________________________loop__________________________________*/

void loop() {
    webSocket.loop();
    delay(0);
}
