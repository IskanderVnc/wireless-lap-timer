#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define CE_PIN   9
#define CSN_PIN 10

int statusLed = 5;
int buzzer = 6;
int triggerPort = 7;
int echoPort = 8;


// WIRELESS COMMUNICATION DETAILS //
const uint64_t pipe = 0xE8E8F0F0E1LL; // Defines the communication channel
RF24 radio(CE_PIN, CSN_PIN); // Sets up the communication
int datatosend[4] = {1,1,9,5};
// ----------------------------- //

void setup() {
pinMode( triggerPort, OUTPUT );
pinMode( echoPort, INPUT );
pinMode( buzzer, OUTPUT );
pinMode(statusLed,OUTPUT);
Serial.begin( 9600 );
radio.begin();
radio.openWritingPipe(pipe);
}
void loop() {
  digitalWrite(statusLed,HIGH);
//trigger output set to LOW
digitalWrite( triggerPort, LOW );
//send 10microsec impulse on trigger
digitalWrite( triggerPort, HIGH );
delayMicroseconds( 10 );
digitalWrite( triggerPort, LOW );
long duration = pulseIn( echoPort, HIGH );
long r = 0.034 * duration / 2;
Serial.println(r);
// If distance detected is within specified range : Object detected => Activate buzzer for audio feedback and send data to receiver;
if( r>=40 && r<=100){
Serial.println("END LAP");
//enableBuzzer();
// SET UP HERE THE DATA TRANSMISSION TO RECEIVER //
  	signalLapCompletionToReceiver();
}
delay(10);
}

// FUNCTION ENABLING BUZZER WITH A TIMEOUT OF 5 SECONDS
unsigned long previousMillis = 0;  // will store last time the buzzer has been enabled
const long interval = 5000; 

void enableBuzzer(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you enabled the buzzer
    previousMillis = currentMillis;

  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer,LOW);

  }
}

void signalLapCompletionToReceiver(){
	radio.write (datatosend, sizeof(datatosend)) ;
}