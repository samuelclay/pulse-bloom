// Pulse & Bloom - Ground Lighting
// 2014 - Samuel Clay, samuel@ofbrooklyn.com

// ===========
// = Globals =
// ===========

const uint8_t boardPin = A2;
unsigned long pulseStart = 0;
uint16_t pulseWidth = 1000; // 60 bpm

void clearWires();

// ==========
// = States =
// ==========

typedef enum
{
    STATE_RESTING = 0,
    STATE_PULSE_HIGH = 1,
    STATE_PULSE_LOW = 2
} state_app_t;
state_app_t appState;

// =========
// = Setup =
// =========

void setup() {                
  // The EL channels are on pins 2 through 9
  // Initialize the pins as outputs
  pinMode(2, OUTPUT);  // channel A  
  pinMode(3, OUTPUT);  // channel B   
  pinMode(4, OUTPUT);  // channel C
  pinMode(5, OUTPUT);  // channel D    
  pinMode(6, OUTPUT);  // channel E
  pinMode(7, OUTPUT);  // channel F
  pinMode(8, OUTPUT);  // channel G
  pinMode(9, OUTPUT);  // channel H
  // We also have two status LEDs, pin 10 on the Escudo, 
  // and pin 13 on the Arduino itself
  pinMode(10, OUTPUT);     
  pinMode(13, OUTPUT);    
  
  // The main board drives the pin HIGH for 1/10th of a pulse's bpm
  pinMode(boardPin, INPUT);
  
  appState = STATE_RESTING;
}

// ========
// = Loop =
// ========

void loop() {
    int pulseFound = (millis() % 1000 < 100) || digitalRead(boardPin);

    if (appState == STATE_RESTING) {
        if (pulseFound) {
            pulseStart = millis();
            appState = STATE_PULSE_HIGH;
        }
    }
    
    if (appState == STATE_PULSE_HIGH && !pulseFound) {
        pulseWidth = millis() - pulseStart;
    }
    
    if (appState == STATE_PULSE_HIGH || appState == STATE_PULSE_LOW) {
        clearWires();
        long elapsed = millis() - pulseStart;
        if (elapsed > pulseWidth * 4) {
            appState = STATE_RESTING;
        } else if (elapsed > pulseWidth * 3) {
            digitalWrite(9, LOW);
            digitalWrite(8, LOW);
        } else if (elapsed > pulseWidth * 2) {
            digitalWrite(7, LOW);
            digitalWrite(6, LOW);
        } else if (elapsed > pulseWidth * 1) {
            digitalWrite(5, LOW);
            digitalWrite(4, LOW);
        } else if (elapsed > pulseWidth * 0) {
            digitalWrite(3, LOW);
            digitalWrite(2, LOW);
        }
    }
}

void clearWires() {
    for (int i=2; i <= 9; i++) {
        digitalWrite(i, HIGH);
        i++;
    }
}