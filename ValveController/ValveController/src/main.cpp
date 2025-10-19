#include <Wire.h>
#include <FastAccelStepper.h>
#include <Adafruit_MCP23X08.h> // Use Adafruit library for the I/O expander
#include <vector>
#include <valve.hpp>
#include <sstream>

// Pin definitions for the microcontroller
#define STEP_PIN 2
#define DIR_PIN 3
#define SDA_PIN 8  // Custom SDA pin
#define SCL_PIN 10 // Custom SCL pin

// MCP23008 I2C address (A0=1, A1=1, A2=1)
#define MCP23008_ADDR 0x27

// MCP23008 pin definitions for clarity
#define VALVE_COUNT 6      // Valves are on pins 0 through 5
#define BUTTON_PULSE_PIN 6    // Button to trigger a short pulse on all valves

int selectedValve = -1; // Describes what valve is being chosen for an action

enum mode {
  AWAIT, // When no action is happening
  PUSH, // When a push action is being made
  TIMER, // When a set Timer action is being made
  UPDATE, // When the device count is being updated
  STAT, // When the status of a valve is being requested
}; 

// Create an instance of the MCP23008
Adafruit_MCP23X08 mcp;
std::vector<ValveController> valves;

mode currentMode = mode::AWAIT;

// FastAccelStepper objects
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

// Motor control variables
const long motorSpeed = 5000;      // steps per second
const long acceleration = 5000;  // steps per second squared

// Button state tracking for debouncing / edge detection
int lastPulseButtonState = HIGH;

void setup() {
  Serial.begin(115200);

  // Initialize I2C with custom pins BEFORE initializing the MCP23008 library
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize the MCP23008
  if (!mcp.begin_I2C(MCP23008_ADDR, &Wire)) {
    while (1);
  }

  // Configure MCP23008 pins
  // Set pins 0-5 (valves) as OUTPUT
  for (int i = 0; i < VALVE_COUNT; i++) {
    mcp.pinMode(i, OUTPUT);
  }
  // Set pin 6 as INPUT with internal pull-up resistors
  mcp.pinMode(BUTTON_PULSE_PIN, INPUT_PULLUP);
  
  // Initialize valve objects
  for (int i = 0; i < VALVE_COUNT; i++) {
    valves.push_back(ValveController(&mcp, i+1));
  }

  // Initialize FastAccelStepper engine
  engine.init();

  // Create stepper instance and start continuous rotation
  stepper = engine.stepperConnectToPin(STEP_PIN);
  if (stepper) {
    stepper->setDirectionPin(DIR_PIN);
    stepper->setSpeedInHz(motorSpeed);
    stepper->setAcceleration(acceleration);
    stepper->runForward();
  } 
}

void turnAllValvesOn() {
  for (ValveController& valve : valves) {
    valve.activateValve();
  }
}

void handleButton() {
  // --- Handle the Pulse Button (GP6) ---
  int currentPulseButtonState = mcp.digitalRead(BUTTON_PULSE_PIN);
  if (currentPulseButtonState == LOW) { turnAllValvesOn();}
}

void handleSerialInput() {
  if (Serial.available()) {
    char receivedChar = Serial.read();

    switch (receivedChar) {
      case 'P':
        currentMode = mode::PUSH;
        break;
      case 'S':
        currentMode = mode::STAT;
        break;
      case 'U':
        currentMode = mode::UPDATE;
        break;
      case 'T':
        currentMode = mode::TIMER;
      default:
        break;
    }
  }
}

void handlePush() {
  if (Serial.available()) {
    char receivedChar = Serial.read();
    bool activated = true;
    int id;

    switch (receivedChar) {
      case 'A':
        for (ValveController & controller : valves) {
          activated = activated && controller.isAvailable();
        }
        turnAllValvesOn();
        Serial.print(activated);
        break;
      default:
        id = receivedChar - '0';
        activated = valves[id-1].activateValve();
        Serial.print(activated);
        break;
    }

    currentMode = mode::AWAIT;
  }
}

void handleStatus() {
  if (Serial.available()) {
    char receivedChar = Serial.read();

    std::stringstream ss;
    std::string s;

    switch (receivedChar) {
      case 'A':
        for (ValveController & controller : valves) {
          if (controller.isActive()) ss << controller.isAvailable() << ',';
        }
        s = ss.str();
        s.pop_back();
        Serial.println(s.c_str());
        break;
      default:
        int id = receivedChar - '0';
        bool available = valves[id-1].isAvailable();
        Serial.print(available);
        break;
    }

    currentMode = mode::AWAIT;
  }
}

void handleUpdate() {
  if (Serial.available()) {
    char receivedChar = Serial.read();

    int valveAmount = receivedChar - '0';

    for (int i = 0; i < VALVE_COUNT; i++) {
      valves[i].toggleValve(i < valveAmount ? true : false);
    }

    Serial.print(valveAmount + '0');
    currentMode = mode::AWAIT;
  }
}

void handleTimer() {
  if (Serial.available()) {
    char receivedChar = Serial.read();
    String number = "";
    if (selectedValve != -1) number = Serial.readStringUntil('\n');

    if (number.isEmpty()) {
      switch (receivedChar) {
        case 'A':
          selectedValve = 0;
          break;
        default:
          selectedValve = receivedChar - '0';
          break;
      }
    }
    else {
      int newTimeout = atoi(number.c_str());
      Serial.println(number);

      switch (selectedValve) {
        case 0:
          for (ValveController & valve : valves) {
            valve.setTimeout(newTimeout);
          }
          break;
        default:
          valves[selectedValve-1].setTimeout(newTimeout);
          break;
      }

      selectedValve = -1;
      currentMode = mode::AWAIT;
    }
  }
}

void loop() {
  // Check for button presses and update valve states
  handleButton();

  switch (currentMode) {
    case AWAIT:
      handleSerialInput();
      break;
    case PUSH:
      handlePush();
      break;
    case TIMER:
      handleTimer();
      break;
    case UPDATE:
      handleUpdate();
      break;
    case STAT:
      handleStatus();
      break;
  }
}