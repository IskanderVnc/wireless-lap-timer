int triggerPort = 7;
int echoPort = 8;
int buzzer = 9;
unsigned long time;
unsigned long lampeggio_time;
unsigned long pausa_time;
void setup() {
pinMode( triggerPort, OUTPUT );
pinMode( echoPort, INPUT );
pinMode( buzzer, OUTPUT );
Serial.begin( 9600 );
Serial.println( "Sensore ultrasuoni: ");
}
void loop() {
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
enableBuzzer();
}
delay(10);
}

// FUNCTION IMPLEMENTING BLINKING RED LED WITHOUT delay()
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 5000; 

void enableBuzzer(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you enabled the LED
    previousMillis = currentMillis;

  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer,LOW);

  }
}

