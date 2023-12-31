#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

#define CE_PIN   9
#define CSN_PIN 10

int redLedPin = 16;
int greenLedPin = 15;
int buzzerPin = 17;
int photoresistorPin = A0;

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);



// BUTTON 1-2 WITH DEBOUNCING : BUTTON 1 = CALIBRATE ; BUTTON 2 = START//
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

int laserLightThreshold = 750; 
bool firstStart = true;
bool isCalibrated = false;
int detectedLight = 0;
int calibrating = false;
int systemStatus = 0;
bool printFlag = true;
bool goneThrough = false;
bool buzzerEnabled = true;
int buzzerState = LOW;

const uint64_t pipe = 0xE8E8F0F0E1LL; // Defines the communication channel
RF24 radio(CE_PIN, CSN_PIN); // Sets up the communication
int datatoreceive[4];
float lastComputedLap = -1;

uint8_t checkmarkSymbol[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};

void setup() {
  // put your setup code here, to run once:
  lcd.createChar(0, checkmarkSymbol); // create a new custom character (index 0)
  lcd.createChar(1, clock); // create a new custom character (index 1)
  pinMode(redLedPin,OUTPUT);
  pinMode(greenLedPin,OUTPUT);
  pinMode(button1Pin,INPUT);
  pinMode(button2Pin,INPUT);
  pinMode(buzzerPin, OUTPUT);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
  Serial.begin(9600);
  lcd.begin(16, 2);
  //lcd.print("Timer Lap!");
}

void loop() {
  // put your main code here, to run repeatedly:
  detectedLight = analogRead(photoresistorPin);
  Serial.println(detectedLight);
  if (isCalibrated && systemStatus != 2){
    readButtonWithDebouncing(button2Pin,button2State,lastButton2State);
    digitalWrite(redLedPin, LOW);  
  } else {
	  if(!isCalibrated){
    digitalWrite(redLedPin, HIGH);
	  } else digitalWrite(redLedPin,LOW);
    readButtonWithDebouncing(button1Pin,button1State,lastButton1State);
  }
  Serial.print("SystemSTATUS :");
  Serial.println(systemStatus);
  switch (systemStatus) { 	
    case 0: 
      // CASE 0 : IDLE ( Start lap not pressed or computation terminated / no calibration in progress or calibration just terminated)
      radio.flush_rx();
      if(isCalibrated == true && firstStart == false){
      lcdPrintFirstLine("Press 'START'"); 
      lcdPrintSecondLine(isCalibrated,-1);
      digitalWrite(greenLedPin,HIGH);
      } else if(isCalibrated == false && firstStart == true){
        lcdPrintFirstLine("Press 'CALIBRATE' and align laser");
        lcdPrintSecondLine(isCalibrated,-1);
        digitalWrite(redLedPin,HIGH);
      } else if(isCalibrated == false && firstStart == false){
        lcdPrintFirstLineNoScrolling("CALIBRATION LOST !");
        lcdPrintSecondLine(isCalibrated,-1);
      }
      Serial.println("IDLE");
      checkCalibration(detectedLight);
      if(lastComputedLap != -1){
        //Serial.println(lastComputedLap);
        lcd.clear();
        lcdPrintSecondLine(isCalibrated,lastComputedLap);
      }
      buzzerEnabled = true;
      buzzerState = LOW;
			break;

		case 1: 
      // CASE 1 : CALIBRATION IN PROGRESS
      lcdPrintFirstLineNoScrolling("CALIBRATION...");
      calibrateLaser(detectedLight);
			break; 

		case 2: 
      // CASE 2 : LAP CALCULATION IN PROGRESS
      //Serial.println("LAP CALCULATION");
      if(goneThrough == false){
			lcdPrintFirstLine("Ready to start timer");
      } else {
        lcdPrintFirstLine("... Lap in progress ...");
      }
      readButtonWithDebouncing(button2Pin,button2State,lastButton2State);
      startLapMeasurement(detectedLight);
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
        if(buttonPin == 19){
          if(systemStatus != 2){
            systemStatus = 2;
            goneThrough = false;
           digitalWrite(buzzerPin, HIGH);
        delay(150);
        digitalWrite(buzzerPin, LOW);
          } else {
            Serial.println("SETTING SYS STATUS TO 0 AFTER BUTTON PRESSURE");
            systemStatus = 0;
        digitalWrite(buzzerPin, HIGH);
        delay(150);
        digitalWrite(buzzerPin, LOW);
          }
        lcd.clear();
        } else  {
          systemStatus = !systemStatus;
        digitalWrite(buzzerPin, HIGH);
        delay(150);
        digitalWrite(buzzerPin, LOW);
        lcd.clear();
        }
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
  while(millis() - start < 3000){
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
      firstStart = false;
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
  
  if(lightLevel < laserLightThreshold && isCalibrated == true) {
    long start = millis();
    bool calibrationCheck = false;
    while(millis() - start < 1500){
       long readLaser = analogRead(photoresistorPin);
      if (readLaser > laserLightThreshold){
          calibrationCheck = true;
          break;
        }
    }
      if(calibrationCheck == false){
    lcd.clear();
    Serial.println("CALIBRATION LOST!!!");
    lcdPrintFirstLineNoScrolling("LOST CALIBRATION");
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
}


long start = 0;
void startLapMeasurement(int lightLevel){
  blinkingLedWithoutDelay(greenLedPin);
  if (lightLevel < laserLightThreshold && goneThrough == false){
    Serial.println("LAP STARTED AND GONE THROUGH SET TO TRUE");
    buzzerOnOff(buzzerPin);
    goneThrough = true;
    radio.flush_rx();
    start = millis();
  }
  if (goneThrough)
  { 
    Serial.println("GONE THROUGH");
    buzzerOnOff(buzzerPin);
    lcdPrintSecondLine(isCalibrated,float(millis() - start)/float(1000));
    radioComm();
}
}

void radioComm(){
// Add "&& (millis()-start > 5000" in the if - if a minimum threshold is needed
  if ( radio.available() ){
    //Serial.println("RADIO AVAILABLE");
    // leggi i dati in ricezione finchè il messaggio è completo
    bool done = false;
    while (!done){
      // Open message
      radio.read( datatoreceive, sizeof(datatoreceive) );
      if(datatoreceive[0] == 1 && datatoreceive[1] == 1 && datatoreceive[2] == 9 && datatoreceive[3] == 5){
        lastComputedLap =  float(millis() - start)/float(1000);
        goneThrough = false;
        systemStatus = 0;
        done = true;
        digitalWrite(buzzerPin, HIGH);
        delay(200);
        digitalWrite(buzzerPin, LOW);
        delay(200);
        lcd.clear();
      }
    }
  }
}

// VARIABLES BUZZER ON/OFF //

unsigned long previousMillisBuzzer = 0;  // will store last time Buzzer state was updated
const long intervalBuzzer = 800;  // interval at which to on/off buzzer (milliseconds)

void buzzerOnOff(int buzzerPin){
  unsigned long currentMillis;
  if(buzzerEnabled){
    //Serial.println(" AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA BUZZER ENABLED TRUE AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    currentMillis = millis();
  if (currentMillis - previousMillisBuzzer >= intervalBuzzer) {
    // save the last time you enabled the buzzer
    previousMillisBuzzer = currentMillis;

    // if the buzzer is off turn it on and vice-versa:
    if (buzzerState == LOW) {
      Serial.println("SETTING BUZZER TO HIGH");
      buzzerState = HIGH;
    } else {
      Serial.println("SETTING BUZZER TO LOW");
      buzzerEnabled = false;
      buzzerState = LOW;
    }

    // set the buzzer with the buzzerState of the variable:
    digitalWrite(buzzerPin, buzzerState);
  }
  }
}

// VARIABLES BLINKING LED //
int ledState = LOW;  // ledState used to set the LED
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 500;  // interval at which to blink (milliseconds)

void blinkingLedWithoutDelay(int ledPin){
    unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}

void lcdPrintFirstLine(char* wordPrint){
  lcd.setCursor(0, 0);
  lcd.print(scrollFirstLine(wordPrint));
  delay(150);
}

void lcdPrintFirstLineNoScrolling(char* wordPrint){
  lcd.setCursor(0, 0);
  lcd.print(wordPrint);
  delay(150);
}


void lcdPrintSecondLine(bool isCalibrated, float time){

  if(systemStatus != 2){
	if(!isCalibrated){
      if(lastComputedLap != -1){
      lcd.setCursor(0,1);
      lcd.print("Last lap:");
         lcd.setCursor(10,1);
      lcd.print(lastComputedLap);
      lcd.setCursor(15,1);
      lcd.print("s");
      }else{
      lcd.setCursor(0,1);
      lcd.print("NOT READY");
      }
  } else {
    if(lastComputedLap == -1){
          lcd.setCursor(0,1);
      lcd.print("READY     ");
      lcd.setCursor(6,1);
      lcd.write((byte)0);
    } else {
      lcd.setCursor(0,1);
      lcd.print("READY     ");
      lcd.setCursor(5,1);
      lcd.write((byte)0);
      lcd.setCursor(7,1);
      lcd.print("L:");
      lcd.setCursor(10,1);
      lcd.print(lastComputedLap);
      lcd.setCursor(15,1);
      lcd.print("s");
    }
  }
  } else {
  lcd.setCursor(0, 1);
  lcd.write((byte)1);
  lcd.setCursor(2,1);
  lcd.print(time);
  lcd.setCursor(7,1);
  lcd.print(" s");
  }
}

int i = 16;
int ii = 0;
String scrollFirstLine(String wordDisplay){
  String result;
  String processStr = "                " + wordDisplay + "                ";
  result = processStr.substring(i, ii);
  i++;
  ii++;
  if (i > processStr.length()){
    i = 16;
    ii = 0;
  }
  return result;
}
