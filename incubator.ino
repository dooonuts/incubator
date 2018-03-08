int led = 2;           // the PWM pin the LED is attached to
int heater = 4;

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT);
  pinMode(heater, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // set the brightness of pin 9:
  digitalWrite(led, HIGH);
  digitalWrite(heater, HIGH);
 
  delay(10000);
  
  digitalWrite(led, LOW);
  digitalWrite(heater, LOW);
  
  delay(10000);
}
