/*
 
 */
#include <math.h>
#include "protothreads.h"

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
#define LED GREEN_LED

int outputValue = 0;        // value output to the PWM (analog out)
double tempCelsius = 0;
pt ptBlink;
pt ptTemp;
pt ptSerial;

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
    sensorValue = analogRead(analogInPin); 
    tempCelsius = Thermistor(sensorValue); 
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
    Serial.println("}"); 
    PT_SLEEP(pt, interval);
  }
  PT_END(pt);
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  pinMode(LED, OUTPUT);
  PT_INIT(&ptBlink);
  PT_INIT(&ptTemp);
  PT_INIT(&ptSerial);
}

void loop() {
  // LED loop
  // PT_SCHEDULE(blinkThread(&ptBlink));

  // Temperature reader loop
  PT_SCHEDULE(tempThread(&ptTemp));              

  // Serial printing loop
  PT_SCHEDULE(serialThread(&ptSerial));
                     
}
