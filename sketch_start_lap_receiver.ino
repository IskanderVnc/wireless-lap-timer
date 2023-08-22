int redLedPin = 2;
int blueLedPin = 3;
int greenLedPin = 4;

int photoresistorPin = A0;
int laserLightThreshold = 600;

// BUTTON 1 WITH DEBOUNCING //
int button1Pin = 10;
int button1State = 0;
int lastButton1State = LOW;  // the previous reading from the input pin
int component1State = LOW; // current state of the output pin connected to the button
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
//

void setup() {
  // put your setup code here, to run once:
  pinMode(redLedPin,OUTPUT);
  pinMode(blueLedPin,OUTPUT);
  pinMode(greenLedPin,OUTPUT);
  pinMode(button1Pin,INPUT);
  Serial.begin(9600);
}

int detectedLight = 0;
int calibrating = false;

void loop() {
  // put your main code here, to run repeatedly:
  detectedLight = analogRead(photoresistorPin);

  readButton1WithDebouncing(button1Pin,redLed1Pin);
  // CASE 1 : IDLE ( Start lap not pressed or computation terminated / no calibration in progress or calibration just terminated)

  // CASE 2 : CALIBRATION IN PROGRESS

  // CASE 3 : LAP CALCULATION IN PROGRESS

  
}

void readButton1WithDebouncing(int buttonPin, int componentPin){
 // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButton1State) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != button1State) {
      button1State = reading;

      // only toggle the LED if the new button state is HIGH
      if (button1State == HIGH) {
        component1State = !component1State;
      }
    }
  }
  // set the LED:
  digitalWrite(componentPin, component1State);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButton1State = reading;
}

void calibrateLaser(){
 unsigned long startTime = millis();
   while(millis() - startTime < 30000)
   {
      
   }
}