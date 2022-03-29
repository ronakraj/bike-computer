/*
 
 */
#include <math.h>
#include "protothreads.h"

// These constants won't change.  They're used to give names
// to the pins used:
const int tempInPin = A0;  // Analog input pin that the potentiometer is attached to
const int cadReedPin = A1;
#define LED GREEN_LED

int outputValue = 0;        // value output to the PWM (analog out)
double tempCelsius = 0;
int cadReedState = 1;
int prevCadState = 1;
int timeCounter = 0;
int senseFlipCount = 0;
long int cadRPM = 0;

pt ptBlink;
pt ptTemp;
pt ptSerial;
pt ptCadence;

int blinkThread(struct pt* pt) {
  PT_BEGIN(pt);
  int interval = 100;
  // Loop forever
  for(;;) {
    digitalWrite(LED, HIGH);
    PT_SLEEP(pt, interval);
    digitalWrite(LED, LOW);
    PT_SLEEP(pt, interval);
  }
  PT_END(pt);
}

double Thermistor(int rawADC) {
  double temp;
  temp = log((((40960000/rawADC) - 10000)));
  temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp ))* temp );
  temp = temp - 273.15; // Convert Kelvin to Celcius
  return temp;
}

int tempThread(struct pt* pt) {
  PT_BEGIN(pt);
  int interval = 1000;
  int sensorValue = 0;
  
  // Loop forever
  for(;;) {
    // read the analog in value:
    sensorValue = analogRead(tempInPin); 
    tempCelsius = Thermistor(sensorValue); 
    PT_SLEEP(pt, interval);
  }
  PT_END(pt);
}

int cadThread(struct pt* pt) {
  PT_BEGIN(pt);
  int interval = 1;

  // Loop forever
  for(;;) {
    // Read reed state
    cadReedState = digitalRead(cadReedPin);

    // See if reed switch state was changed (magnet sensed)
    if(prevCadState != cadReedState) {
      senseFlipCount += 1;
    }
    prevCadState = cadReedState;

    timeCounter += 1;

    // Calculate RPM at 1 second interval
    if((timeCounter * interval) >= 1000) {
      cadRPM = (senseFlipCount * 60);
      
      // Reset all counters
      timeCounter = 0;
      senseFlipCount = 0;
    }
    
    PT_SLEEP(pt, interval);
  }
  PT_END(pt);
}

int serialThread(struct pt* pt) {
  PT_BEGIN(pt);
  int interval = 1000;

  for(;;) {
    // print the results to the serial monitor:
    Serial.print("{'temp': ");                       
    Serial.print(tempCelsius);
    Serial.print(", 'cadence': ");
    Serial.print(cadRPM);
    Serial.println("}"); 
    PT_SLEEP(pt, interval);
  }
  PT_END(pt);
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  pinMode(LED, OUTPUT);
  pinMode(tempInPin, INPUT);
  pinMode(cadReedPin, INPUT);
  PT_INIT(&ptBlink);
  PT_INIT(&ptTemp);
  PT_INIT(&ptSerial);
  PT_INIT(&ptCadence);
}

void loop() {
  // LED loop
  // PT_SCHEDULE(blinkThread(&ptBlink));

  // Temperature reader loop
  PT_SCHEDULE(tempThread(&ptTemp));              

  // Cadence reed switch loop
  PT_SCHEDULE(cadThread(&ptCadence));
  
  // Serial printing loop
  PT_SCHEDULE(serialThread(&ptSerial));
                     
}
