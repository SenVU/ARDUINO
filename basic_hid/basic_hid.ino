/*
  program Game Controller as HID

  Upload this file to your Arduino,
  you can check with Notepad / Word or any other text based program
  the output of the joystick button (space)
*/

#include "Keyboard.h"
#include "Mouse.h"

// -------
// DEFINES
// -------
#define NR_OF_ARRAY_ELEMENTS(array) (sizeof(array) / sizeof(typeof(*array)))


// ---------
// CONSTANTS
// ---------
enum pinModes {
  UNUSED_PIN = -1,
  DIGITAL_INPUT = 0,
  ANALOG_INPUT = 1,
  DIGITAL_OUTPUT = 2,
  ANALOG_OUTPUT = 3,
  SERVO = 4,
  DIGITAL_INPUT_PULLUP = INPUT_PULLUP
};

enum pinHidTypes {
  NONE = -1,
  KEY = 0,
  MOUSE = 1,
  KEY_SPAM = 2,
  MOUSE_SPAM = 3,
};

const int JOYSTICK_BUTTON_PIN = 2;
const int LEFT_BUTTON_PIN = 3;
const int UP_BUTTON_PIN = 4;
const int DOWN_BUTTON_PIN = 5;
const int RIGHT_BUTTON_PIN = 6;
const int FIRE2_BUTTON_PIN = 7;
const int FIRE1_BUTTON_PIN = 8;

int PIN_MODES[]{
  UNUSED_PIN,            // pin 00, TX
  UNUSED_PIN,            // pin 01, RX
  DIGITAL_INPUT,         // pin 02, joystick button
  DIGITAL_INPUT,         // pin 03, left button
  DIGITAL_INPUT,         // pin 04, up button
  DIGITAL_INPUT,         // pin 05, down button
  DIGITAL_INPUT,         // pin 06, right button
  DIGITAL_INPUT,         // pin 07, fire 2 button
  DIGITAL_INPUT,         // pin 08, fire 1 button
  DIGITAL_INPUT_PULLUP,  // pin 09, servo, does not properly do PWM if one or more servo pins are used
  ANALOG_OUTPUT,         // pin 10, right led
  ANALOG_OUTPUT,         // pin 11, middle led
  ANALOG_OUTPUT,         // pin 12, left led
  DIGITAL_OUTPUT,        // pin 13, onboard led
  ANALOG_INPUT,          // pin 14, joystick y axis
  ANALOG_INPUT,          // pin 15, joystick x axis
  UNUSED_PIN,            // pin 16/A2,
  UNUSED_PIN,            // pin 17/A3,
  UNUSED_PIN,            // pin 18/A4,
  UNUSED_PIN,            // pin 19/A5,
  UNUSED_PIN,            // pin 20/A6,
  UNUSED_PIN             // pin 21/A7,
};

int PIN_HID_TYPES[]{
  // pin nr, on board function

  NONE,   // pin 00, TX
  NONE,   // pin 01, RX
  KEY,    // pin 02, joystick button
  KEY,    // pin 03, left button
  KEY,    // pin 04, up button
  KEY,    // pin 05, down button
  KEY,    // pin 06, right button
  MOUSE,  // pin 07, right fire button
  MOUSE,  // pin 08, left fire button
  KEY,    // pin 09,
  NONE,   // pin 10, right led
  NONE,   // pin 11, middle led
  NONE,   // pin 12, left led
  NONE,   // pin 13, onboard led
  NONE,   // pin 14/A0, joystick y axis
  NONE,   // pin 15/A1, joystick x axis
  NONE,   // pin 16/A2,
  NONE,   // pin 17/A3,
  NONE,   // pin 18/A4,
  NONE,   // pin 19/A5,
  NONE,   // pin 20/A6,
  NONE    // pin 21/A7,
};

const int MAX_PINS = NR_OF_ARRAY_ELEMENTS(PIN_MODES);


// ----------------
// GLOBAL VARIABLES
// ----------------
int isButtonPressed[MAX_PINS];
int wasButtonPressed[MAX_PINS];
char button2character[MAX_PINS] = {
  0,            //pin 00, TX
  0,            //pin 01, RX
  KEY_TAB,      //pin 02, joystick button
  'a',          // pin 03, left button
  'w',          // pin 04, up button
  's',          // pin 05, down button
  'd',          // pin 06, right button
  MOUSE_LEFT,   // pin 07, right fire button
  MOUSE_RIGHT,  // pin 08, left fire button
  ' ',          // pin 09,
  0,            // pin 10, right led
  0,            // pin 11, middle led
  0,            // pin 12, left led
  0,            // pin 13, onboard led
  0,            // pin 14/A0, joystick y axis
  0,            // pin 15/A1, joystick x axis
  0,            // pin 16/A2,
  0,            // pin 17/A3,
  0,            // pin 18/A4,
  0,            // pin 19/A5,
  0,            // pin 20/A6,
  0             // pin 21/A7,
};

void setup() {
  delay(5000);

  set_all_pinmodes();

  Mouse.begin();
  Keyboard.begin();

  Serial.begin(9600);
}

void set_all_pinmodes() {
  for (int pin = 0; pin < MAX_PINS; pin++) {
    if (PIN_MODES[pin] != UNUSED_PIN) {
      pinMode(pin, PIN_MODES[pin]);
    }
  }
}

void loop() {
  read_all_inputs();
  write_digital_hid_outputs();
  Mouse.move(return_joystick(A1, 1) / 150, return_joystick(A0, 1) / 150);
}

void read_all_inputs() {
  for (int pin = 0; pin < MAX_PINS; pin++) {
    if (PIN_MODES[pin] == DIGITAL_INPUT) {
      wasButtonPressed[pin] = isButtonPressed[pin];
      isButtonPressed[pin] = digitalRead(pin);
    }
    if (PIN_MODES[pin] == DIGITAL_INPUT_PULLUP) {
      wasButtonPressed[pin] = isButtonPressed[pin];
      isButtonPressed[pin] = !digitalRead(pin);
    }
  }
}

int return_joystick(int pin, int deadzone) {
  int toReturn = analogRead(pin) - (1023 / 2);
  if (toReturn < -deadzone) {
    toReturn += deadzone;
  } else if (toReturn > deadzone) {
    toReturn -= deadzone;
  } else {
    toReturn = 0;
  }
  return toReturn;
}

void write_digital_hid_outputs() {
  for (int pin = 0; pin < MAX_PINS; pin++) {
    if (PIN_MODES[pin] == DIGITAL_INPUT || PIN_MODES[pin] == DIGITAL_INPUT_PULLUP) {
      char c = button2character[pin];
      if (isButtonPressed[pin]) {
        if (!wasButtonPressed[pin]) switch (PIN_HID_TYPES[pin]) {
            case MOUSE: Mouse.press(c); break;
            case KEY: Keyboard.press(c); break;
            default: break;
          }
        switch (PIN_HID_TYPES[pin]) {
          case MOUSE_SPAM: Mouse.click(c); break;
          case KEY_SPAM:
            Keyboard.press(c);
            Keyboard.release(c);
            break;
          default: break;
        }
      } else if (!isButtonPressed[pin] && wasButtonPressed[pin]) {
        switch (PIN_HID_TYPES[pin]) {
          case MOUSE: Mouse.release(c); break;
          case KEY: Keyboard.release(c); break;
          default: break;
        }
      }
    }
  }
}
