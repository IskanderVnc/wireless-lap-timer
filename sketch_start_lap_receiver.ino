int redLedPin = 2;
int blueLedPin = 3;
int greenLedPin = 4;
int buzzerPin = 12;
int photoresistorPin = A0;

int laserLightThreshold = 750;

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

bool isCalibrated = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(redLedPin,OUTPUT);
  pinMode(blueLedPin,OUTPUT);
  pinMode(greenLedPin,OUTPUT);
  pinMode(button1Pin,INPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
}

int detectedLight = 0;
int calibrating = false;
int systemStatus = 0;

void loop() {
  // put your main code here, to run repeatedly:
  detectedLight = analogRead(photoresistorPin);
  Serial.println(detectedLight);
  if (isCalibrated)
    digitalWrite(redLedPin, LOW);  
  else{
    digitalWrite(redLedPin, HIGH);
    readButtonWithDebouncing(button1Pin,button1State,lastButton1State);
  }
  
  switch (systemStatus) { 	
		case 0: 
      // CASE 0 : IDLE ( Start lap not pressed or computation terminated / no calibration in progress or calibration just terminated)
			Serial.println("IDLE");
      checkCalibration(detectedLight);
			break;
		case 1: 
      // CASE 1 : CALIBRATION IN PROGRESS
			Serial.println("CALIBRATION IN PROGRESS"); 
      calibrateLaser(detectedLight);
			break; 
		case 2: 
      // CASE 2 : LAP CALCULATION IN PROGRESS
			Serial.println("LAP CALCULATION IN PROGRESS");
			break; 
		} 
  }

void readButtonWithDebouncing(int buttonPin, int &buttonState, int &lastButtonState){
 // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        systemStatus = !systemStatus;
      }
    }
  }
  // set the LED:
  //digitalWrite(componentPin, component1State);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void calibrateLaser(int lightLevel){
 if (lightLevel > laserLightThreshold) {
  long start = millis();
  bool calibrationCheck = true;
  digitalWrite(blueLedPin, HIGH);
  while(millis() - start < 5000){
      long readLaser = analogRead(photoresistorPin);
      if (readLaser < laserLightThreshold){
          calibrationCheck = false;
          break;
        }
    }
   if (calibrationCheck){
      digitalWrite(buzzerPin, HIGH);
      delay(700);
      
      for(int i=0; i<6; i++){
        digitalWrite(blueLedPin, LOW);
        delay(100);
        digitalWrite(blueLedPin, HIGH);
        delay(100);
      }
      digitalWrite(buzzerPin, LOW);
      systemStatus = 0;
      isCalibrated = true;
    }
 }

 else {
  digitalWrite(blueLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  delay(300);
  digitalWrite(redLedPin, LOW);
  delay(300);
  }
}

void checkCalibration(int lightLevel){
  long start = millis();
  if(lightLevel < laserLightThreshold && isCalibrated == true) {
    Serial.println("LOST CALIBRATION");
    digitalWrite(blueLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
    isCalibrated = false;
    
    for(int i=0; i<6; i++){
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(redLedPin, LOW);
      delay(200);
      digitalWrite(buzzerPin, LOW);
      digitalWrite(redLedPin, HIGH);
      delay(100);
    }
  }  
}

/*void readButton1WithDebouncing(int buttonPin){
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
        systemStatus = !systemStatus;
      }
    }
  }
  // set the LED:
  //digitalWrite(componentPin, component1State);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButton1State = reading;
}*/