#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

int redLedPin = 16;
int greenLedPin = 15;
int buzzerPin = 17;
int photoresistorPin = A0;

int laserLightThreshold = 750;

// BUTTON 1 WITH DEBOUNCING //
int button1Pin = 18;
int button1State = 0;
int lastButton1State = LOW;  // the previous reading from the input pin
int button2Pin = 19;
int button2State = 0;
int lastButton2State = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

bool isCalibrated = false;
int detectedLight = 0;
int calibrating = false;
int systemStatus = 0;
bool printFlag = true;
bool goneThrough = false;

const uint64_t pipe = 0xE8E8F0F0E1LL; // Defines the communication channel
RF24 radio(CE_PIN, CSN_PIN); // Sets up the communication
int datatoreceive[4];


void setup() {
  // put your setup code here, to run once:
  pinMode(redLedPin,OUTPUT);
  pinMode(greenLedPin,OUTPUT);
  pinMode(button1Pin,INPUT);
  pinMode(button2Pin,INPUT);
  pinMode(buzzerPin, OUTPUT);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  detectedLight = analogRead(photoresistorPin);
  //Serial.println(detectedLight);
  if (isCalibrated){
    readButtonWithDebouncing(button2Pin,button2State,lastButton2State);
    digitalWrite(redLedPin, LOW);  
  }
  else{
    digitalWrite(redLedPin, HIGH);
    readButtonWithDebouncing(button1Pin,button1State,lastButton1State);
  }
  
  switch (systemStatus) { 	
    case 0: 
      // CASE 0 : IDLE ( Start lap not pressed or computation terminated / no calibration in progress or calibration just terminated)
      Serial.println("IDLE...");  
      checkCalibration(detectedLight);
			break;

		case 1: 
      // CASE 1 : CALIBRATION IN PROGRESS
      Serial.println("CALIBRATION IN PROGRESS");
      calibrateLaser(detectedLight);
			break; 

		case 2: 
      // CASE 2 : LAP CALCULATION IN PROGRESS
			//Serial.println("LAP CALCULATION IN PROGRESS");
      startLapMeasurement(detectedLight);
      radioComm()
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
        if(buttonPin == 19)
          systemStatus = 2;
        else 
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
  digitalWrite(greenLedPin, HIGH);
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
        digitalWrite(greenLedPin, LOW);
        delay(100);
        digitalWrite(greenLedPin, HIGH);
        delay(100);
      }
      digitalWrite(buzzerPin, LOW);
      systemStatus = 0;
      isCalibrated = true;
    }
 }
 else {
  digitalWrite(greenLedPin, LOW);
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
    digitalWrite(greenLedPin, LOW);
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


long start = 0;
void startLapMeasurement(int lightLevel){
  digitalWrite(greenLedPin, LOW);
  delay(500);
  if (lightLevel < laserLightThreshold && goneThrough == false){
    goneThrough = true;
    digitalWrite(greenLedPin, HIGH);
    start = millis();
  } 
  Serial.println(millis() - start);
}


void radioComm(){

  /* Basato sull'esempio "YourDuinoStarter Example: nRF24L01 Transmit Joystick values"
 http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
 http://www.danielealberti.it/2016/01/ricetrasmettitori-rf-per-arduino.html
 Collegamenti del modulo RF24L01 ad Arduino:
 1 - GND
 2 - VCC 3.3V !!! NOT 5V
 3 - CE to Arduino pin 9
 4 - CSN to Arduino pin 10
 5 - SCK to Arduino pin 13
 6 - MOSI to Arduino pin 11
 7 - MISO to Arduino pin 12
 8 - UNUSED
 */

  if ( radio.available() ){
    Serial.println("RADIO AVAILABLE");
    // leggi i dati in ricezione finchè il messaggio è completo
    bool done = false;
    while (!done){
      // ricevi il messaggio
      radio.read( datatoreceive, sizeof(datatoreceive) );
      if(datatoreceive[0] == 1){
        Serial.println("DATA TO RECEIVE :");
      Serial.print(datatoreceive[0]);
      Serial.print(" ");
            Serial.print(datatoreceive[1]);
      Serial.print(" ");
            Serial.print(datatoreceive[2]);
      Serial.print(" ");
            Serial.print(datatoreceive[3]);
      Serial.println(" ");
      }
    }
  }
}