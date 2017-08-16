/*TeensyBaseUnit  MJL thepiandi.blogspot.com    August 16, 2017

Sketch for a TeensyLC programmed as a Midi USB.  This device is called
the Base Unit Teensy Device.

The purpose of this sketch is to have accelerometers in a handheld
device change a note played on a Midi device. 

This sketch, along with two ESP8266 sketchs form a basis that can be 
expanded to provide some interesting musical effects.  

The handheld device (actually called Hand-held device) contains
an ESP8266 microcontroller that sends the X and Y position of a MPU9250
Inertial Measurement Unit to another ESP8255 in the base unit, the Base 
Unit WiFi Defice.  This relays the same data to the Teensy in this device
via a simple serial connection. Teensy is required because it provides 
a true USB MIDI interface.

X and Y positions of the Hand-held device of -90.0 to +90.0 degrees are 
reduced to values between -24 and +24. The X position represents
49 notes, or four octaves.  A value of zero represents middle C.
The 49 values of the Y position, are used to control the velocity.  A 
value of zero represents a velocity of 64. or in the middle of the range.
A place holder in the data is included for rotation about the Z direction.  
That will be included at a latter time.

The data trasmitted to the Teensy is in the following format:
Sxxx,yyy,zzzE followed by CR and LF.  Where xxx and yyy are -24 to 24.
Right now zzz is an arbitary value of -15, and is not used.

The onboard LED on the Teensy is toggled everytine a message is received
from the client ESP.  This is just a sanity check to make sure everything
is working.
 
*/

/***********************************global*************************/
int led = 13;
int note;
int velocity;
int Z_value;

/*********************************ledControl***********************/
void ledControl(){
  //switches LED every acquisition of data from Serial Interface 
  boolean static ledOnOff;
  
  if (ledOnOff){
    digitalWrite(led, LOW);
    ledOnOff = false;
  }

  else{
    digitalWrite(led, HIGH);
    ledOnOff = true;
  }
}

/*__________________________________setup__________________________________*/
void setup() {
  pinMode(led, OUTPUT);
  Serial1.begin(115200);
  
  note = 0;
  velocity = 0;
  
  //send middle C at half velocity
  usbMIDI.sendNoteOn((60 + note), (64 + velocity), 1);
}

/*__________________________________loop__________________________________*/
void loop() {
  int charIn;
  String  totalMessage;
  boolean messDone;
  int index1, index2, index3;
  int newNote, newVelocity;

  totalMessage = "";
  
  //Only one character is received while Serial1 is available
  messDone = false;
  while(!messDone){
    while(Serial1.available() > 0){
      charIn = Serial1.read();
      if (charIn == 10){
        messDone = true;
      }
      totalMessage += String(char(charIn));
    }
  }

  if (totalMessage.startsWith("S")){
    ledControl();  //cycle LED

    index1 = totalMessage.indexOf(",");
    index2 = totalMessage.indexOf(",", index1 + 1);
    index3 = totalMessage.indexOf("E");
    newNote = totalMessage.substring(1, index1).toInt();
    newVelocity = totalMessage.substring(index1 + 1, index2).toInt();
    Z_value = totalMessage.substring(index2 + 1, index3).toInt();

    if (newNote != note){
      usbMIDI.sendNoteOff((60 + note), (64 + velocity), 1);
      usbMIDI.sendNoteOn((60 + newNote), (64 + newVelocity), 1); 
      note = newNote;
      velocity = newVelocity;         
    }
  }  
}
