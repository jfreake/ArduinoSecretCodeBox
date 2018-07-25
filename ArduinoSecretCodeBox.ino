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
  https://github.com/jfreake/ArduinoSecretCodeBox
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
const String  patternB    = "brbbrr";  // Target pattern starting with the left/black button
const String  patternR    = "rbrrbb";  // Target pattern starting with the right/red button
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
bool throwError       = false;   // used to trigger an error and reset of the current pattern



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


  Serial.println("Playing startup tune...It's-a-Me"); 
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 13; thisNote++) {

    // to calculate the note duration, take one second divided by the note type. ie quarter note = 1000 / 4, eighth note = 1000/8
    int noteDuration = 1000 / durationAry[thisNote];

    if ( melodyAry[thisNote] != 0 ){
      analogWrite(ledGreenPin, 255); // Blink with the tune
    }
    tone(tonePin, melodyAry[thisNote], noteDuration);


    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes/2);
    analogWrite(ledGreenPin, 0);
    delay(pauseBetweenNotes/2);
    // stop the tone playing:
    noTone(tonePin);
  }

  // Lock the Box and detach so it's not using power
  Serial.println("Locking Box");
  analogWrite(ledRedPin, 255);
  mainServo.write(0);
  // Wait half a sec for the motor to get to it's position
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

    // If one of the buttons were pressed, start the evaluation of the pattern
    if (currentChar != ""){
      // If we are coming off of a successful lock, relock everything and reset if another button has been pressed.
      if (locked == false){
      Serial.println("Re-Locking");
      mainServo.attach(servoPin);
      mainServo.write(0);
      delay(500);
      mainServo.detach();  
      locked = true;
      }
      
      // light up light
      analogWrite(ledRedPin, 0);
      analogWrite(ledGreenPin, 255);
      // Beep
      tone(tonePin, NOTE_G5, 1000/128);
      analogWrite(ledGreenPin, 0);

      // Increment the button count
      buttonPressCounter++;

      // Append button press to the current pattern
      currentPattern = currentPattern + String(currentChar);
      Serial.println(String(currentChar) + " button pressed");
      Serial.println("matching number " + String(buttonPressCounter) + " in pattern ");
      Serial.println("Current pattern:" + String(currentPattern));

      //Check if current pattern matches either target pattern
      if(patternB.startsWith(currentPattern)|| patternR.startsWith(currentPattern)){
        // all good...lets keep going
        Serial.println("All Good");
      }else{
        //nope...it didn't match, set the trigger to reset and start over
        throwError = true;
      }

      // reset and start over
      if (throwError == true){
        Serial.println("ERROR...resetting");
        analogWrite(ledRedPin, 255);
        tone(tonePin, NOTE_G4, 1000/16);
        delay(1000/16);
        analogWrite(ledRedPin, 0);
        delay(1000/16);
        analogWrite(ledRedPin, 255);
        tone(tonePin, NOTE_G4, 1000/16);
        delay(1000/16);
        analogWrite(ledRedPin, 0);
        delay(1000/16);
        analogWrite(ledRedPin, 255);
        tone(tonePin, NOTE_G4, 1000/2);
        currentPattern = "";
        buttonPressCounter = 0;
        throwError = false;
      }

      // Check to see if current pattern matches fully and set unlock variable if so.
      if (patternR == currentPattern || patternB == currentPattern){
        locked = false;
      }

      // Congrats...you unlocked the box with one of the target patterns
      if (locked == false /* pattern is matched */){
        analogWrite(ledRedPin, 0);
        analogWrite(ledGreenPin, 255);
        Serial.println("Open Lock");
        mainServo.attach(servoPin);
        mainServo.write(128);
        delay(500);
        mainServo.detach();
        currentPattern = "";
        buttonPressCounter = 0;
        throwError = false;
      }
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("button released");
    }
    // Delay a little bit to avoid bouncing
    delay(150);
  }




  
  // save the current state as the last state, for next time through the loop
  lastBlackState = buttonBlackState;
  lastRedState = buttonRedState;  
}

// This was fun...thanks to Mr. Streeter at https://www.creativekids.info/ for the challenge and the materials.
// This was my first experience with an Arduino board and it won't be my last.
