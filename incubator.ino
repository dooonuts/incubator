int heater = 4;
int pinA0 = pinA0;
int current = 0;
int adc;
// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  Serial.begin(9600);
  pinMode(pinA0, INPUT);
  pinMode(heater, OUTPUT);
  current = 1;
}

// the loop routine runs over and over again forever:
void loop() {
  /* Switches cases are from 1-4. 1- Default(Start-Up/Set-up, Will likely include calibration) 2- Heating 3- Cooling 4- Finished */
  if(current>=1 and current<=4)
  {
    adc = analogRead(pinA0);
    Serial.println(adc);
    switch(current)
    {
      case 1:
        Serial.println('1');
         current = 2;
         break;
      case 2:
        Serial.println('2');
        current = 3;
        break;
      case 3:
        Serial.println('3');
        current = 4;
        break;
      case 4:
        Serial.println('4');
        current = 1; /* This is only here for now because I'm not sure what we're going to do with it */
        break;
    }
  }
  Serial.println("Here");
  delay(1000);
}
