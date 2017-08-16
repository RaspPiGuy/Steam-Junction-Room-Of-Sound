/*ESPbaseUnit  MJL thepiandi.blogspot.com    August 16, 2017

This sketch is for an ESP8266 microcintroller that receives data 
wirelessly from the Hand-held device and transmitts the same data to 
a Teensy microprocessor. Both this device, the Base Unit WiFi device
and the Teensy device, Base Unit Teensy, sit on the "base unit" that connects
via USB to a Midi output device.  

The purpose is to have accelerometers in the Hand-held device change 
a note played on a Midi output device. The idea is that the Hand-held 
device will have no cable connections. It will be battery powered so 
as not to be tethered to anything else.

The ESP8266 in this device is programmed to connect, via WiFi, to the Hand-held
device.  Once connected, it connects to the Hand-held's URL.  The two cevices
then start to exchange data.  The method of transmission is via TCP/IP Sockets.
Sockets allow the connection between devices to remain open unless delibertly
closed, and allows for bidirectional communication with virtually no latenct.  

The timing of message transmission is controlled by this Base Unit WiFi device.  
Once one device sends data over the Socket, it waits for a response from the 
other.  The Hand-held device makes its measurements and calculations and sends
its data to the Base Unit WiFi device.  This device receives the data and 
processes it.  The Base Unit WiFi device establishs the measurement rate which 
is approximately 10 measurements per second. This is accomplished by having it
wait 100 ms. after processing its data and before responding back to the 
Hand-held device.  This is accurate enough for the present.  However, if the 
gyroscope positions are to be included, a more accurate method of timing using 
micros() or millis().

The Base Unit WiFi device does nothing with the data it receives except to re-
transmit that data to the Base Unit Teensy device.  The method of transmission 
to the Teensy is via a simple serial connection at Serial1.

It has been noted that the ESP8266 can go into a RESET condition.  After adding
the delay(0) in the loop() function seems to have solved that problem.

The blue LED on the ESP flashes five times when a connection is made between the
Hand-held device and the Base Unit WiFi device.

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

/*********************************webSocketEvent***********************/
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == 3){
    for (int i = 0; i < length; i++){
      Serial1.print(char(payload[i]));
    }
  }
  
  delay(100);
  webSocket.sendTXT("OK");
}

/*__________________________________setup__________________________________*/
void setup() {
  //Serial.begin(115200);
  //Serial Connection to Teensy
  Serial1.begin(115200);
  
  //Setting up LED
  pinMode(LED, OUTPUT);

  //turn LED Off
  digitalWrite(LED, HIGH);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  //flash LED
  for (int i = 0; i < 5; i++){
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
  }

  delay(1000);

  //Setup Websocket
  webSocket.begin(host, httpPort);
  webSocket.onEvent(webSocketEvent);

}

/*__________________________________loop__________________________________*/
void loop() {
  webSocket.loop();
  delay(0);
}
