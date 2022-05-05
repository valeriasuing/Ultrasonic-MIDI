//Valeria Suing & Angelina Do 2022
//Code for sending MIDI Messages to Computer using an ultrasonic sensor and two push buttons 
//Code referenced Gustavo Silveira's code https://github.com/silveirago/Fliper-2/blob/master/Code/Fliper/Fliper.ino

#include "MIDIUSB.h" //library for Arduino Micro to use for MIDI messages

//ULTRASONIC SENSOR 
#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR04

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

int sensorCS = 0; //current state of sensor
int sensorPS = 0; //previous state of sensor

int midiCS = 0; //current state of MIDI
int midiPS = 0; //previous state of MIDI

//Buttons
const int buttons = 2; //number of buttons
const int buttonsPins[buttons] = {10, 11}; // Pins connected to buttons 

int buttonsCS[buttons] = {}; //Current state of buttons 
int buttonsPS[buttons] = {}; //Previous state of buttons 

//deals with contact chatter 
unsigned long lastDebounceTime[buttons] = {0}; 
unsigned long debounceDelay = 50; 

// MIDI Assignments 
byte midiCh = 1; // MIDI channel to be used
byte cc = 1; // MIDI CC to be used for sensor
const int ccButtons[buttons] = {20,21}; //MIDI CC for buttons

int buttonPresses = 0; //variable to identify how many times the button was pressed

void setup() {
  Serial.begin(31250);
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT

  //pull-up resistors for buttons
   for (int i = 0; i < buttons; i++) {
    pinMode(buttonsPins[i], INPUT_PULLUP);
  }
}

void loop() {
  // starting the distance sensor
  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2);  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);   // getting the time difference between sending and receiving
  distance = (duration/2) / 29.1;     //converting time to cm

  sensorCS = distance; 
  midiCS = map(sensorCS, 0, 127, 0, 40); //127 is the highest CC value
  
  if (distance>5 && distance<40) {
    if (midiPS != midiCS) {
       controlChange(midiCh, cc, midiCS); //  (channel, CC number,  CC value)
       MidiUSB.flush(); 
       sensorPS = sensorCS; //update / reset values
       midiPS = midiCS; 
    }
  }

  //loop for all buttons 
  for(int i=0; i < buttons; i++) {
    buttonsCS[i] = digitalRead(buttonsPins[i]); //read PINS from arduino
    //contact chatter
    if((millis() - lastDebounceTime[i]) > debounceDelay) {
      if(buttonsPS[i] != buttonsCS[i]) { 
        lastDebounceTime[i] = millis(); 

        //if button is pressed 
        if(buttonsPS[i] == LOW) {
          buttonPresses++; //if button is pressed add one to buttonPresses variable
          boolean oddOrEven = buttonPresses % 2; //identify if it's even or odd

          if(oddOrEven){ //if the button wass pressed one time
            controlChange(midiCh, ccButtons[i], 127); //CC with a value of 127
             MidiUSB.flush();
            Serial.print(i);
            Serial.println(": button on");
          }
          //send MIDI CC number 
          if (!oddOrEven) { //if the button was pressed a second time
            controlChange(midiCh, ccButtons[i], 0); //CC with a value of 0 
            MidiUSB.flush();
          }
        } else {
          MidiUSB.flush(); 
        }
        buttonsPS[i] = buttonsCS[i]; //resets
      }
    }
  }
}

//Control change function
void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
