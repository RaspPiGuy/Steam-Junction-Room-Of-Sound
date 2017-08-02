/*MPU2ESP2  MJL thepiandi.blogspot.com    August 1, 2017

Sketch for a TeensyLC programmed as a Midi USB

The purpose of this sketch is to have a rotated gyro in a handheld
device, change a note played on a Midi device. 

This sketch, along with two ESP8266 sketchs form a basis that can be 
expanded to provide some interesting musical effects.  The output
of this sketch is certainly not an interesting musical effect, just
a starting point.

A handheld ESP8266 sends rotation data from a MPU9250 to a client
ESP8266.  The client ESP8266 sends a number between 1 and 6 (followed
by CR and LF) to this sketch which controls a TeensyLC.  The TeensyLC
provides a true USB MIDI interface. 

The client ESP connects to the Teensy via: ESP Serial1 (pin TXD1)to 
Teensy Serial1 (pin RX1).

This sketch simply sends a note to the USB Midi output. Whenever the 
Server ESP is rotated in the X direction, the frequency of the note 
is increased and a new note is played. Whenever the Server ESP is 
rotated in the Y direction, the frequency of the note is decreased
and a new note is played. 

The onboard LED on the Teensy is toggled everytine a message is received
from the client ESP.  This is just a sanity check to make sure everything
is working.
 
*/

/***********************************global*************************/
int led = 13;
int note;

/*********************************ledControl***********************/
void ledControl(){
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
//  Serial.begin(115200);
  Serial1.begin(115200);
  note = 0;
  usbMIDI.sendNoteOn((60 + note), 0x7F, 1);

}

/*__________________________________loop__________________________________*/
void loop() {
  int charIn;
  int action;
  boolean static everyOther;
  
  while(Serial1.available() > 0){
    charIn = Serial1.read();
    if ((charIn > 48) && (charIn < 55)){
      action = charIn - 48;
      
      if (everyOther){
        usbMIDI.sendNoteOff((60 + note), 0x7F, 1);
        if ((action == 1) || (action == 4)){
          note++;
        }
        else if ((action == 2) || (action == 5)){ 
          note--;  
        }
        usbMIDI.sendNoteOn((60 + note), 0x7F, 1);
      }
      everyOther = !everyOther; 
    }             

    if (charIn == 10){
      ledControl();
    }
  } 
}
