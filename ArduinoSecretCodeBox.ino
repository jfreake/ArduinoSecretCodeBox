/*
  ArduinoSecretCodeBox
  aka (Paradiddle-diddle box code)

  Box unlocks when the the buttons are pressed in a certain sequence :)

  circuit:
  - Black Push Button on digital pin 2
  - Red Push Button on digital pin 3
  - Green LED on digital pin 8
  - Red LED on digital pin 9
  - 8 ohm speaker on digital pin 10
  - Servo motor on analog pin 0 (pin 14)

  created 2018-07-21
  modified 2018-07-21
  by Jason Freake (jfreake@iframe.ca)
*/

// Includes
#include "pitches.h"
#include <Servo.h>


// Constants won't change:
const int  buttonBlackPin = 2;   // the digital pin that the pushBlack is attached to
const int  buttonRedPin   = 3;   // the digital pin that the pushRed is attached to
const int  ledGreenPin    = 8;   // the digital pin that the green LED is attached to
const int  ledRedPin      = 9;   // the digital pin that the red LED is attached to
const int  tonePin        = 10;  // the digital pin that is connected to the tone buzzer
const int  servoPin       = 14;  // the analog pin pin that controls the lock servo motor
const int  melodyAry[]    = { NOTE_E5, NOTE_E5, 0, NOTE_E5, 0, NOTE_C5, NOTE_E5, 0, NOTE_G5, 0, 0, 0, NOTE_G4 }; // array of notes to be played
const int  durationAry[]  = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };                                           // array of duration of notes to be played
const String  patternB    = "brbbrr";
const String  patternR    = "rbrrbb";
Servo mainServo;

// Variables will change:
int buttonPressCounter    = 0;   // counter for the number of button presses
int buttonBlackState      = 0;   // current state of the black button
int buttonRedState        = 0;   // current state of the red button
int lastBlackState     = HIGH;   // previous state of the black button
int lastRedState       = HIGH;   // previous state of the red button
String currentChar       = "";   // the char associated with the current button press
String currentPattern    = "";   // holds the current pattern being assembled
bool locked            = true;   // holds the value that states if the box should currently be locked
bool throwError       = false;   // holds



// Initialize everything
void setup() {
  // init serial communication:
  Serial.begin(9600);
  Serial.println("Serial initialized");
  
  // Init the servo pin
  mainServo.attach(servoPin);
  Serial.println("Servo initialized");
  
  // init button pins as a input_pullup - bypasses the need for resistors
  pinMode(buttonBlackPin, INPUT_PULLUP);
  pinMode(buttonRedPin, INPUT_PULLUP);
  Serial.println("Buttons initialized");

  // init LED pins
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledRedPin, OUTPUT); 
  Serial.println("LEDs initialized");


  Serial.println("Playing startup tune"); 
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 13; thisNote++) {

    // to calculate the note duration, take one second divided by the note type. ie quarter note = 1000 / 4, eighth note = 1000/8
    int noteDuration = 1000 / durationAry[thisNote];

    if ( melodyAry[thisNote] != 0 ){
      analogWrite(ledGreenPin, 255); // Blink with the tune
    }
    // tone(tonePin, melodyAry[thisNote], noteDuration);


    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes/2);
    analogWrite(ledGreenPin, 0);
    delay(pauseBetweenNotes/2);
    // stop the tone playing:
    noTone(tonePin);
  }

  
  Serial.println("Locking Box");
  analogWrite(ledRedPin, 64);
  mainServo.write(0);
  delay(500);
  mainServo.detach(); 

  Serial.println("Starting Main Loop, Watching for button presses");
}


// Start the main loop
void loop() {

  // read the pushbutton input pin:
  buttonBlackState = digitalRead(buttonBlackPin);
  buttonRedState = digitalRead(buttonRedPin);



  

  // compare the buttonState to its previous state
  if (buttonBlackState != lastBlackState || buttonRedState != lastRedState) {
    // find out which button was pressed
    if (buttonBlackState == LOW){
      currentChar = "b";
    }else{
      if(buttonRedState == LOW){
      currentChar = "r";  
      }else{
        currentChar = "";
      }
    }

    if (currentChar != ""){
      // light up light
      analogWrite(ledGreenPin, 255);
      // Beep
      tone(tonePin, NOTE_G5, 1000/128);
      
      buttonPressCounter++;
      currentPattern = currentPattern + String(currentChar);
      Serial.println(String(currentChar) + " button pressed");
      Serial.println("matching number " + String(buttonPressCounter) + " in pattern ");
      Serial.println("Current pattern:" + String(currentPattern));

      //Check pattern matches
      if(patternB.startsWith(currentPattern)|| patternR.startsWith(currentPattern)){
        // all good
        Serial.println("All Good");
      }else{
        throwError = true;
      }

      if (throwError == true){
        Serial.println("ERROR...resetting");
        tone(tonePin, NOTE_G4, 1000/32);
        delay(1000/32);
        tone(tonePin, NOTE_G4, 1000/32);
        delay(1000/32);
        tone(tonePin, NOTE_G4, 500);
        currentPattern = "";
        buttonPressCounter = 0;
        throwError = false;
      }

      if (patternR == currentPattern || patternB == currentPattern){
        locked = false;
      }

      if (locked == false /* pattern is matched */){
        Serial.println("Open Lock");
        mainServo.attach(servoPin);
        mainServo.write(128);
        delay(500);
        mainServo.detach();
        locked == true;
      }
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("button released");
      analogWrite(ledGreenPin, 0);
    }
    // Delay a little bit to avoid bouncing
    delay(150);
  }




  
  // save the current state as the last state, for next time through the loop
  lastBlackState = buttonBlackState;
  lastRedState = buttonRedState;  
}
