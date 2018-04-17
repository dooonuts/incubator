// include the library code:
#include <Wire.h>
#include <Math.h>
#include <MatrixMath.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define CONSTA 0.001125308852122
#define CONSTB 0.000234711863267
#define CONSTC 0.000000085663516
#define TEMPREADERPIN 4
#define HEATERPIN 9
#define THERMISTORPIN 14 /* This should be A0 */
#define ADC_RANGE 1024.0
#define RESISTOR 9980.0

/* If we ever want colors */
#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

uint8_t buttons;
uint8_t digits=0;
bool screen_changed;
unsigned long last_time_pressed;
unsigned long last_time_sent;
unsigned long debounce_time=250;
int vd_resistor = 10000; /* Value of the resistance of the voltage divider */
int current = 0;
int cal_counter = 0;
int select = 0;
int bounds = 0;

double temp_bounds[2];
int screen_digits[3];

double adc = 0;
double temperature=0.0;

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Enter Lower: ");

  pinMode(TEMPREADERPIN, OUTPUT);
  pinMode(THERMISTORPIN, INPUT);
  pinMode(HEATERPIN, OUTPUT);
  last_time_pressed = millis();
  last_time_sent = millis();
  current = 1;
  screen_changed = true;
}

// the loop routine runs over and over again forever:
void loop() {
  /* Switches cases are from 1-4. 1- Set Bounds 2- Heating 3- Cooling 4- Finished */
  if(current>=1 and current<=4)
  {
    buttons = lcd.readButtons();
    buttonCheck();
    switch(current)
    {
      case 1:
        /* If lower bound is selected */
        if(bounds == 0 && select == 1)
        {
          storeBound();
          current = 2;
          select = 0;
        }
        break;
      case 2:          
        /* If upper bound is selected */
        if(bounds > 0 && select == 1)
        {
          storeBound();
          current = 3;
        }
        break;
      case 3:
         readTemp();
         boundCheckUP();
        break;
      case 4:
        readTemp();
        boundCheckDOWN();
        break;
    }
  }
}

/* I did not write this, only for debugging purposes */
void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
       frac = (val - int(val)) * precision;
   else
       frac = (int(val)- val ) * precision;
   Serial.println(frac,DEC) ;
} 

/* Converts Voltage to Resistance */
double voltageConversion()
{
  double resistance = (double)(ADC_RANGE/adc)*RESISTOR - RESISTOR;
  return resistance;
}

/* Converts Resistance to Temperature */
double steinhart(){
  double curr_resistance = voltageConversion();
  double tempR = log(curr_resistance);
  double temp = 1/(CONSTA + (CONSTB*tempR) + (CONSTC*tempR*tempR*tempR))-273;
  return temp;
}

void incrementDigits(int temp_digits)
{
  if(screen_digits[temp_digits] == 9)
  {
    if(temp_digits<2)
    {
      screen_digits[temp_digits] = 0;
      temp_digits++;
      incrementDigits(temp_digits);
    }
  }
  else
  {
    screen_digits[temp_digits] = screen_digits[temp_digits] + 1;
  }
}

void decrementDigits(int temp_digits)
{
  if(screen_digits[temp_digits] == 0)
  {
    if(temp_digits<2)
    {
      screen_digits[temp_digits] = 9;
      temp_digits++;
      decrementDigits(temp_digits);
    }
  }
  else
  {
    screen_digits[temp_digits] = screen_digits[temp_digits] - 1;
  }
}
/* Checks for pressed buttons */
void buttonCheck() {
   if(buttons && (millis()-last_time_pressed)>debounce_time)
   {
    last_time_pressed = millis();
    if (buttons & BUTTON_UP) {
      incrementDigits(digits);
      screen_changed = true;
    }
    if (buttons & BUTTON_DOWN) {
      decrementDigits(digits);
      screen_changed = true;
    }
    if (buttons & BUTTON_LEFT) {
      if(digits < 2)
      {
        digits++;
      }
      else
      {
        digits = 2;
      }
    }
    if (buttons & BUTTON_RIGHT) {
      if(digits > 0)
      {
        digits--;
      }
      else
      {
        digits = 0;
      }
    }
    if (buttons & BUTTON_SELECT) {
      select = 1;
    }
   }
   if(screen_changed == true)
   {
      printScreenDigits();
      screen_changed = false;
   }
}

void storeBound() {
  temp_bounds[bounds] = (double)screen_digits[0]*1/10 + screen_digits[1] + screen_digits[2]*10;
  bounds++;
  if(bounds == 1)
  {
    lcd.setCursor(0,0);
    lcd.print("Enter Upper: ");
    clearScreenDigits();
  }
  else if(bounds == 2)
  {
    Serial.println("Lower bound is: ");
    printDouble(temp_bounds[0],1000);
    Serial.println("Upper bound is: ");
    printDouble(temp_bounds[1],1000);
    clearScreenDigits();
    lcd.setCursor(0,0);
    lcd.print("Temperature is: ");
    digitalWrite(TEMPREADERPIN, HIGH);
    heaterON();
  }
}

void boundCheckUP() {
  if(temperature > temp_bounds[1])
  {
    heaterOFF();
    current = 4;
  }
}

void boundCheckDOWN() {
  if(temperature < temp_bounds[0])
  {
    heaterON();
    current = 3;
  }
}

void readTemp() {
  adc = analogRead(THERMISTORPIN);
  temperature = steinhart();
  if(millis()-last_time_sent>1000)
  { 
    last_time_sent = millis();
    Serial.println("Input Voltage is : ");
    Serial.println(adc);
    Serial.println("Temperature is : ");
    printDouble(temperature, 1000);
    lcd.setCursor(0,1);
    lcd.print(temperature);
    
  }
}

//void setScreenDigits(double unrounded)
//{
//  double rounded = round(unrouded); 
//}

void printScreenDigits(){
  lcd.setCursor(0,1);
  lcd.print(screen_digits[2]);
  lcd.print(screen_digits[1]);
  lcd.print(".");
  lcd.print(screen_digits[0]);
}

void clearScreenDigits(){
  screen_digits[0] = 0;
  screen_digits[1] = 0;
  screen_digits[2] = 0;
}

void heaterON() {
  digitalWrite(HEATERPIN, HIGH);
}

void heaterOFF() {
  digitalWrite(HEATERPIN, LOW);
}



