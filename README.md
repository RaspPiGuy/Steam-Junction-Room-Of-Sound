# Midi Input Device Responding To Commands From A Remote Device

The purpose is to contribute to Steam Junction's Wall of Sound project with a hand-held, non-tethered, device, that sends Midi Commands in response to movement, rotation, and/or position of the device.  The Midi commands will control an output device to make music or sounds. An alternative to making sounds would be to control lights of some nature.

## Present Progress
A prototype of the hand-held device is constructed.  It consists of an ESP8266 (a Node-MCU development board with an ESP-12) and a MPU-9250 9 axis IMU (Inertial Measurement Unit with three axis gyro, three axis Accelerometer, and a three axis magnetometer). For completion, it only requires a source of power and an enclosure.  

A prototype of a Midi input device is constructed.  The hand-held device and the Midi input device communicate with each other.  At this point, the Midi input prototype does not source Midi commands, but three LEDs are controlled by rotation of the hand-held device.

Using the Arduino IDE sketches for both the devices are written.

The devices were demonstrated at the Microcontroller Group meeting at Steam Junction on July 20.  

## Next Step
The Midi input device (as well as the hand-held device) has a USB connector as part of the Node-MCU development board.  

Next step is to determine if it is possible to directly connect that device to the Raspberry Pi Midi Distributor.  

## Communications Between the Hand-held Device and the Midi input device
### At this point in time
The ESP8266 on both devices are WiFi enabled.  

The hand-held device is setup as an access point, creating its own network.  No other network connection is necessary.  The hand-held device is configured as a server while the other device is a client.  

TCP/IP sockets are utilized for communication.  Sockets are a lower order component of the TCP/IP stack.  HTTP commands (commands you use to communicate with web pages, etc.) are at the top of the stack in the application layer while sockets are down two layers in the internet layer.  

Why talk about stacks and layers?  My first communication method was to use HTTP by sending GET commands.  It worked OK but the latency was terrible.  The problem was that the connection closed after each command and had to be reestablished.  Socket communication had no perceivable latency and just worked fine with a minimum of code. 

Once the server (hand-held device) is powered, it establishes the access point, which has its own IP address, then, establishes socket communications at a defined port number.  It starts to look for a client connecting to it.  

When the client is powered, it connects to the server's network.  Once this connection is made, it blinks the LEDs five times.  Socket communications are established at the server's IP address and port number.  It then starts to look for messages from the server.

The server "sees" the client connect and responds by sending a message to turn all LEDs off, then looks for a message back from the client.  When the client sees that message, it turns the LEDs off (they already should be off), and sends an "OK" message back to the server.  

The server responds by measuring the rotation of all three gyro axis and seeing if any measurements are above a threshold of 100 deg per second.  If so, a comparison is made of the three measurements.  A message is returned to the client that depends upon the axis with the greatest rotation and the direction of rotation.  The server then looks for a message from the client.

The client responds to the message by turning on or off the LED corresponding to the gyro axis with the greatest rotation.  The client waits a short time corresponding to the measurement rate of the gyros, and sends the "OK".  

This handshaking works well, and the connection stays open the whole time.


