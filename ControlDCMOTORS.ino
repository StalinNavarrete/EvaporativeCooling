//Creado por Lenin Navarrete, Diego Arellano
//Escuela Politécnica Nacional - Facultad de Ingeniería Mecánica

//Librerías
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

//Ventilador y Bomba
#define fan 11
#define pump 3

int svalue;
int RPM = 0;
int ciclos = 1;
int cycle = 1;
float airSpeed = 0;

//Input & Button Logic
const int numOfInputs = 4;
const int inputPins[numOfInputs] = {6, 5, 4, 7}; //enter,down,up,pump
int inputState[numOfInputs];
int lastInputState[numOfInputs] = {LOW, LOW, LOW, LOW};
bool inputFlags[numOfInputs] = {LOW, LOW, LOW, LOW};
long lastDebounceTime[numOfInputs] = {0, 0, 0, 0};
long debounceDelay = 10;

//LCD Menu Logic
const int numOfScreens = 8;
int currentScreen = 0;
String screens[numOfScreens][2] = {{"Set Fan Speed", "View Data"}, {"Fast", "Medium"},
  {"Low", "Off"}, {"Return", ""}, {"RPM:", "AirSpeed:"}, {"Return", ""}, {"Set Pump Cycles", ""}, {"No. cycles:", "Return"}
};
int cursorPosition = 0;
int fanSpeed = -1;
long ms_from_start;
long ms_interval;
long ms_pump_on;
long ms_interval_on;
long ms_pump_off;
long ms_interval_off;

//Vector for arrow
uint8_t arrow[8] = {0x0, 0x04 , 0x06, 0x1f, 0x06, 0x04, 0x00, 0x00};

void setup()
{
  pinMode(fan, OUTPUT);
  digitalWrite(fan, LOW);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, LOW);

  for (int i = 0; i < numOfInputs; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  lcd.begin();
  lcd.createChar(0, arrow);   //Create the arrow symbol
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("WELCOME!");
  delay(2000);
}

void loop() {
  setInputFlags();
  resolveInputFlags();
  resolveOutputs();
}

//Funciones:::::

void setInputFlags() {
  for (int i = 0; i < numOfInputs; i++) {
    int reading = digitalRead(inputPins[i]);
    if (reading != lastInputState[i]) {
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != inputState[i]) {
        inputState[i] = reading;
        if (inputState[i] == HIGH) {
          inputFlags[i] = HIGH;
        }
      }
    }
    lastInputState[i] = reading;
  }
}

void resolveInputFlags() {
  for (int i = 0; i < numOfInputs; i++) {
    if (inputFlags[i] == HIGH) {
      inputAction(i);
      inputFlags[i] = LOW;
      printScreen();
    }
  }
}

void inputAction(int input) {
  long startTime = millis();
  if (input == 0 && startTime > 3500) { //enter
    if (currentScreen == 0 && cursorPosition == 0) {
      currentScreen = 1;
      cursorPosition = 0;
    } else if (currentScreen == 1 && cursorPosition == 0) {
      fanSpeed = 0;
    } else if (currentScreen == 1 && cursorPosition == 1) {
      encendido();
      fanSpeed = 1;
    } else if (currentScreen == 2 && cursorPosition == 0) {
      encendido();
      fanSpeed = 2;
    } else if (currentScreen == 2 && cursorPosition == 1) {
      fanSpeed = 3;
    } else if (currentScreen == 3 && cursorPosition == 0) {
      currentScreen = 0;
    }
    if (currentScreen == 0 && cursorPosition == 1) {
      currentScreen = 4;
      cursorPosition = 0;
    } else if (currentScreen == 5 && cursorPosition == 0) {
      currentScreen = 0;
    }
    if (currentScreen == 6 && cursorPosition == 0) {
      currentScreen = 7;
      cursorPosition = 0;
    } else if (currentScreen == 7 && cursorPosition == 1) {
      currentScreen = 0;
      cursorPosition = 0;
    } else if (currentScreen == 7 && cursorPosition == 0) {
      cursorPosition = 2;
    } else if (currentScreen == 7 && cursorPosition == 2) {
      cursorPosition = 0;
      cycle = ciclos;
    }
  }
  else if (input == 1 && startTime > 3500) { //down
    if (cursorPosition == 0 && (currentScreen != 3 && currentScreen != 5 && currentScreen != 4 && currentScreen != 6)) {
      cursorPosition = 1;
    } else if (cursorPosition == 1 && currentScreen == 0) {
      currentScreen = 6;
      cursorPosition = 0;
    } else if (cursorPosition == 1 && currentScreen == 1) {
      currentScreen = 2;
      cursorPosition = 0;
    } else if (cursorPosition == 1 && currentScreen == 2) {
      currentScreen = 3;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 3) {
      currentScreen = 1;
      cursorPosition = 0;
    } else if ((cursorPosition == 1 || cursorPosition == 0) && currentScreen == 4) {
      currentScreen = 5;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 5) {
      currentScreen = 4;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 6) {
      currentScreen = 0;
      cursorPosition = 0;
    } else if (cursorPosition == 1 && currentScreen == 7) {
      cursorPosition = 0;
    } else if (cursorPosition == 2 && currentScreen == 7) {
      parameterChange(0);
    }
  }
  else if (input == 2 && startTime > 3500) { // up
    if (cursorPosition == 1 && (currentScreen != 3 && currentScreen != 5 && currentScreen != 4)) {
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 0) {
      currentScreen = 6;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 1) {
      currentScreen = 3;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 2) {
      currentScreen = 1;
      cursorPosition = 1;
    } else if (cursorPosition == 0 && currentScreen == 3) {
      currentScreen = 2;
      cursorPosition = 1;
    } else if ((cursorPosition == 1 || cursorPosition == 0) && currentScreen == 4) {
      currentScreen = 5;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 5) {
      currentScreen = 4;
      cursorPosition = 0;
    } else if (cursorPosition == 0 && currentScreen == 6) {
      currentScreen = 0;
      cursorPosition = 1;
    } else if (cursorPosition == 0 && currentScreen == 7) {
      cursorPosition = 1;
    } else if (cursorPosition == 2 && currentScreen == 7) {
      parameterChange(1);
    }
  }
  else if (input == 3 && startTime > 3500) { // pump
    digitalWrite(fan, LOW);
    for (int i = 1; i <= cycle; i++) {
      pump_on();
      pump_off();
    }
    if (svalue != 0) {
      encendido();
    }
  }
}
void encendido() {// Torque inicial
  digitalWrite(fan, HIGH);
  ms_from_start = millis();
  ms_interval = 1000;
  while (true) {
    if (millis() - ms_from_start > ms_interval) {
      break;
    }
  }
}

void pump_on() {
  ms_pump_on = millis();
  ms_interval_on = 1500;
  while (true) {
    if (millis() - ms_pump_on > ms_interval_on) {
      digitalWrite(pump, HIGH);
      break;
    }
  }
}

void pump_off() {
  ms_pump_off = millis();
  ms_interval_off = 6000;
  while (true) {
    if (millis() - ms_pump_off > ms_interval_off) {
      digitalWrite(pump, LOW);
      break;
    }
  }
}

void parameterChange(int key) {
  if (key == 0 && ciclos > 1 && ciclos <= 6) { //down
    ciclos--;
  } else if (key == 1 && ciclos >= 1 && ciclos < 6) { //up
    ciclos++;
  } else if (key == 0 && ciclos == 1) { //down
    ciclos = 6;
  } else if (key == 1 && ciclos == 6) { //up
    ciclos = 1;
  }
}

void resolveOutputs() {
  switch (fanSpeed) {
    case 0: //Fast
      digitalWrite(fan, HIGH);
      RPM = 3000;
      airSpeed = 3.2;
      break;
    case 1: //Medium
      svalue = 150;
      analogWrite(fan, svalue); /// value between 0 - 255
      RPM = 1800;
      airSpeed = 1.8;
      break;
    case 2: //Low
      svalue = 40;
      analogWrite(fan, svalue); /// value between 0 - 255
      RPM = 500;
      airSpeed = 0.4;
      break;
    case 3: //Off
      svalue = 0;
      digitalWrite(fan, LOW);
      RPM = 0;
      airSpeed = 0;
      break;
    default:
      break;
  }
}

void printScreen() {
  lcd.clear();
  if ((cursorPosition == 1 || cursorPosition == 0) && currentScreen == 4) {
    lcd.setCursor(0, 0);
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(5, 0);
    lcd.print(RPM);
    lcd.setCursor(0, 1);
    lcd.print(screens[currentScreen][1]);
    lcd.setCursor(10, 1);
    lcd.print(airSpeed);
    lcd.setCursor(13, 1);
    lcd.print("m/s");
  } else if (cursorPosition == 1 && currentScreen == 7) {
    lcd.setCursor(1, 0);
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(13, 0);
    lcd.print(ciclos);
    lcd.setCursor(0, cursorPosition);
    lcd.write(0);
    lcd.print(screens[currentScreen][1]);
  } else if (cursorPosition == 0 && currentScreen == 7) {
    lcd.setCursor(0, cursorPosition);
    lcd.write(0);
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(13, 0);
    lcd.print(ciclos);
    lcd.setCursor(1, 1);
    lcd.print(screens[currentScreen][1]);
  } else if (cursorPosition == 1 && currentScreen != 4) {
    lcd.setCursor(1, 0);
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(0, cursorPosition);
    lcd.write(0);
    lcd.print(screens[currentScreen][1]);
  } else if (cursorPosition == 0 && currentScreen != 4) {
    lcd.setCursor(0, cursorPosition);
    lcd.write(0);
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(1, 1);
    lcd.print(screens[currentScreen][1]);
  } else if (cursorPosition == 2) {
    lcd.setCursor(1, 0);
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(1, 1);
    lcd.print(screens[currentScreen][1]);
    lcd.setCursor(13, 0);
    lcd.write(0);
    lcd.print(ciclos);
  }
}
