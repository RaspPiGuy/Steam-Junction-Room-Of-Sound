# Midi Input Device Responding To Commands From A Remote Device

The purpose is to contribute to Steam Junction's Room of Sound project with a hand-held, non-tethered, device, that sends Midi Commands in response to movement, rotation, and/or position of the device.  The Midi commands will control an output device to make music or sounds. An alternative to making sounds would be to control lighting.

## Progress
### First Stage - Proof Of Concept of Communications Between Two Microcontrollers
A prototype of the Hand-held device is constructed.  It consists of an ESP8266 (a Node-MCU development board with an ESP-12) and a MPU-9250 9 axis IMU (Inertial Measurement Unit with three axis gyro, three axis Accelerometer, and a three axis magnetometer). For completion, it only requires a source of power and an enclosure.  

A prototype of a Midi Base device, also with a Node-MCU development board with an ESP-12, is constructed.  The hand-held device and the Midi Base device communicate with each other.  At this point, the Midi base prototype does not source Midi commands, but three ob-board LEDs are controlled by rotation of the Hand-held device.

Using the Arduino IDE, sketches for both the devices are written.

The devices were demonstrated at the Microcontroller Group meeting at Steam Junction on July 20, 2017.  

### Next Step
The Midi Base (as well as the Hand-held device) has a USB connector as part of the Node-MCU development board.  

Next step is to determine if it is possible to directly connect the Mide base to the Raspberry Pi Midi Distributor.  

### Communications Between the Hand-held Device and the Midi Base device
The ESP8266s on both devices are WiFi enabled.  

The Hand-held device is setup as an access point, creating its own network, with address (SSID) and password. The hand-held device is configured as a server while the other device is a client.  

TCP/IP sockets are utilized for communication.  Sockets are a lower order component of the TCP/IP stack.  HTTP commands (commands you use to communicate with web pages, etc.) are at the top of the stack in the application layer while sockets are down two layers in the internet layer.  

The first communication metho tried was to use HTTP by sending GET commands.  It worked OK but the latency was terrible.  The problem was that the connection closed after each command was sent and had to be reestablished.  Socket communications have no perceivable latency and just worked fine with a suprising minimum of code. 

Once the Hand-held device is powered, it establishes the access point, which has its own IP address.  Next, socket communications are established at a defined port number.  The Hand-held device starts to look for a client that is connecting to it.  

When the Midi Base is powered, it connects to the server's network because it is the client.  Once WiFi connection is made, it blinks all three LEDs, five times.  Socket communications are established at the server's IP address and port number.  To signal to the user that communications are established it blinks all three LEDs on and off five times.  The Midi base is then ready to look for messages from the Hand-held device.

The Hand-held device "sees" the Midi Base connect and responds by sending a message to turn all LEDs off, then looks for a message back from the Midi Base.  When the Midi Base sees that message, it turns the LEDs off (they already should be off), and sends an "OK" message back to the server.  

Whenever the Hand-held receives the "OK" message, it responds by measuring the rotation of all three gyro axis and seeing if any measurements are above a threshold of 100 deg per second.  If so, a comparison is made of the three measurements.  A message is returned to the client that depends upon the axis with the greatest rotation and the direction of rotation.  The server then looks. again, for an "OK" message from the client.

The Midi Base responds to the message by turning on or off the LED corresponding to the gyro axis with the greatest rotation.  The client waits a short time corresponding to the measurement rate of the gyros, and sends the "OK".  

This handshaking works well, and the connection stays open the whole time.

### Second Stage - Sending Midi Commands and Making Some Sounds

Answering the question asked by the first stage regarding the possiblility to directly connect the Mide base to the Raspberry Pi Midi Distributor:  No, the USB of the ESP8266 device cannot be used to connect to the Midi Distributor!   The USB of the ESP8266 imnplements a UART function over USB.  A microcontroller that implements a true USB-Midi is required.

There are no changes made to the Hand-held device.  The same Arduino IDE sketch is used for the Hand-held as is used in the first stage, Proof of Concept.

The Midi Base, on the other hand is completely changed.  The three LEDs are removed and a Teensy-LC microcontroller is added.  The Teensy-LC implements the USB-Midi interface and can also be programmed by the Arduino IDE. The Midi Base's ESP8266 (the Node-MCU development board with an ESP-12) connects to the Teensy-LC using only one connection: TXD1 on the ESP8266 side to RX1 on the Teensy-LC. We are transmitting on the Serial1 output of the ESP8266 to the Serial1 input of the Teensy-LC.  

The Arduino IDE sketch for the Midi Base is altered.  Whenever a message is sent from the Hand-held device, it is simply sent to the Teensy (a simple Serial1.println(... ) is used.  The Teensy-LC, codmposes the Midi command and sends it over the USB MIDI port.  

The sounds made, at this stage of development are not very exciting or entertaining.  A rotation of the X-Axis, in either direction, of the Hand-held results the note played increasing in pitch.  Rotation of the Y-Axis, in either direction, of the Hand-held results the note played decreasing in pitch.  Rotation of the Z-Axis results in the same note being played.  

The Teensy-LC is powered from the USB interface.  The ESP8266 gets its power from the same source via a connection on the Teensy-LC.

## Next Steps
It is important to verify that the Midi Base will interface with the Midi-Distributor correctly.

Next development involves making more interesting sounds.





