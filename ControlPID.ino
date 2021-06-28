//Creado por Lenin Navarrete, Diego Arellano
//Escuela Politécnica Nacional - Facultad de Ingeniería Mecánica
// Librerias
#include "max6675.h"
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "DHT.h"
#include <PID_v1.h>

#define DHTPIN1 36     // Sensor de humedad 1
#define DHTTYPE DHT22   // DHT 22

#define DHTPIN2 34     // Sensor de humedad 2
#define DHTTYPE DHT22   // DHT 22

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
#define DETECT 2  //zero cross detect
#define GATE 10    //triac gate

//Termocupla tipo K 1
int thermoDO1 = 7;
int thermoCS1 = 6;
int thermoCLK1 = 5;
MAX6675 thermocouple2(thermoCLK1, thermoCS1, thermoDO1);
int vccPin1 = 4;
int gndPin1 = 3;

//Termocupla tipo K 2
int thermoDO = 32;
int thermoCS = 30;
int thermoCLK = 28;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
int vccPin = 26;
int gndPin = 24;
int i = 0;

int t1 = 0;
int t2 = 0;


#define PIN_INPUT A0 //LM35 1
#define PIN_OUTPUT 11 
#define PULSE 4   //trigger pulse width (counts)

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//PARAMETROS PID
double Kp = 200, Ki = 9, Kd = 2.5;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Degree symbol

uint8_t degree[8] = {0x1C, 0x14, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00};

void setup()
{
  lcd.begin(); 
  lcd.createChar(0, degree);   //Create the arrow symbol
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("EVAPORATIVE"); //PANTALLA DE BIENVENIDA
  lcd.setCursor(3, 1);
  lcd.print("COOLING SYSTEM");
  lcd.setCursor(13, 3);
  lcd.print("EPN-FIM");
  
  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(DETECT, INPUT);     //zero cross detect
  digitalWrite(DETECT, HIGH);
  pinMode(GATE, OUTPUT);      //triac gate control
  dht1.begin();
  dht2.begin();
  Serial.begin(9600);
  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);
  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);

  pinMode(vccPin1, OUTPUT); digitalWrite(vccPin1, HIGH);
  pinMode(gndPin1, OUTPUT); digitalWrite(gndPin1, LOW);
  pinMode(11, OUTPUT);
  //initialize the variables we're linked to
  Input = analogRead(PIN_INPUT);
  Setpoint = 25;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  OCR1A = 100;      //initialize the comparator
  TIMSK1 = 0x03;    //enable comparator A and overflow interrupts
  TCCR1A = 0x00;    //timer control registers set for
  TCCR1B = 0x00;    //normal operation, timer disabled


  // set up zero crossing interrupt
  attachInterrupt(0, zeroCrossingInterrupt, RISING);
  //IRQ5 is pin 18. Call zeroCrossingInterrupt
  //on rising signal
  delay(2500);
}
//Interrupt Service Routines

void zeroCrossingInterrupt() { //zero cross detect
  TCCR1B = 0x04; //start timer with divide by 256 input
  TCNT1 = 0;   //reset timer - count from zero
}

ISR(TIMER1_COMPA_vect) { //comparator match
  digitalWrite(GATE, HIGH); //set triac gate to high
  TCNT1 = 65536 - PULSE;    //trigger pulse width
}

ISR(TIMER1_OVF_vect) { //timer1 overflow
  digitalWrite(GATE, LOW); //turn off triac gate
  TCCR1B = 0x00;          //disable timer stopd unintended triggers
}


void loop()
{
  t1 = analogRead(A0); //LM35 TEMP1
  t1 = t1 / 2.05;
  t2 = analogRead(A1); //LM35 TEMP2
  t2 = t2 / 2.05;
  Input = thermocouple.readCelsius();
  for (int x = 1; x < 10; x++) { ///////////////PID
    myPID.Compute();
    i = map(Output, 0, 255, 480, 60);
    OCR1A = i;     //set the compare register temperature desired.
  }

  if (digitalRead(23) == LOW) { //RESTAR
    Setpoint--;
  }
  if (digitalRead(22) == LOW) { //SUMAR
    Setpoint++;
  }

  float h1 = dht1.readHumidity();
  float h2 = dht2.readHumidity();

  Serial.print("T1");
  Serial.println(Input);
  Serial.print("T2");
  Serial.println(thermocouple2.readCelsius());
  Serial.print("H1");
  Serial.println(h1);
  Serial.print("H2");
  Serial.println(h2);
  Serial.print("ST");
  Serial.println(Setpoint);
  Serial.print("W1");
  Serial.println(t1);
  Serial.print("W2");
  Serial.println(t2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Temp:  "); lcd.print(Setpoint, 1); lcd.print("");lcd.write(0); lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("T.Hot:     ");  lcd.print(Input, 1); lcd.print("");lcd.write(0); lcd.print("C");
  lcd.setCursor(0, 2);
  lcd.print("T.Cold:    ");  lcd.print(thermocouple2.readCelsius(), 1); lcd.print("");lcd.write(0); lcd.print("C");
  lcd.setCursor(0, 3);
  lcd.print("HR1:");  lcd.print(h1, 1); lcd.print("%");
  lcd.print("  HR2:");  lcd.print(h2, 1); lcd.print("%");
  delay(300);
}
