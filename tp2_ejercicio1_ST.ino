//KIM,MORAT,GUZMAN,NISIM
#include <ESP32Time.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

// Pines
#define SW1 35
#define SW2 34
#define DHTPIN 23
#define DHTTYPE DHT11

#define P1 50
#define ESPERA1 51
#define P2 52
#define ESPERA2 53
int estado = P1;

#define LIBRE 54
#define PULSADO 55
#define ESPERA_SUELTA 56
int estadoSW1 = LIBRE;
int estadoSW2 = LIBRE;

// Objetos
ESP32Time rtc;
DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// Variables de hora seteada
int setHora = 0;
int setMin = 0;

void setup() {
  Serial.begin(115200);

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);

  dht.begin();
  display.begin();
  rtc.setTime(0, 0, 0, 25, 4, 2025); 
}

void loop() {
  float temp = dht.readTemperature();
  if (isnan(temp)) temp = 0.0;

  switch (estado) {
    case P1:
      mostrarPantalla1(temp);
      if (digitalRead(SW1) == LOW && digitalRead(SW2) == LOW) {
        estado = ESPERA1;
      }
      break;

    case ESPERA1:
      if (digitalRead(SW1) == HIGH && digitalRead(SW2) == HIGH) {
        setHora = rtc.getHour(true);
        setMin = rtc.getMinute();
        estado = P2;
      }
      break;

    case P2: 
      mostrarPantalla2();
      actualizarBotones();
      if (digitalRead(SW1) == LOW && digitalRead(SW2) == LOW) {
        estado = ESPERA2;
      }
      break;

    case ESPERA2:
      if (digitalRead(SW1) == HIGH && digitalRead(SW2) == HIGH) {
        rtc.setTime(0, setMin, setHora, 11, 4, 2025);  // segundos, min, hora, día, mes, año
        estado = P1;
      }
      break;
  }
}

void actualizarBotones() {
  int lecturaSW1 = digitalRead(SW1);
  int lecturaSW2 = digitalRead(SW2);

  // SW1: Hora
  switch (estadoSW1) {
    case LIBRE:
      if (lecturaSW1 == LOW && lecturaSW2 == HIGH) estadoSW1 = PULSADO;
      break;
    case PULSADO:
      if (digitalRead(SW1)==HIGH){
      setHora = (setHora + 1) % 24;
      estadoSW1 = ESPERA_SUELTA;
      } else if(digitalRead(SW2)==LOW){
        estadoSW1 == LIBRE;
      }
      break;
    case ESPERA_SUELTA:
      estadoSW1 = LIBRE;
      break;
  }

  // SW2: Minuto
  switch (estadoSW2) {
    case LIBRE:
      if (lecturaSW2 == LOW && lecturaSW1 == HIGH) estadoSW2 = PULSADO;
      break;
    case PULSADO:
      if (digitalRead(SW2)==HIGH){
      setMin = (setMin + 1) % 60;
      estadoSW2 = ESPERA_SUELTA;
      } else if(digitalRead(SW1)==LOW)
      break;
    case ESPERA_SUELTA:
      estadoSW2 = LIBRE;
      break;
 
}
}


void mostrarPantalla1(float temp) {
  char horaStr[6];
  sprintf(horaStr, "%02d:%02d", rtc.getHour(true), rtc.getMinute());

  char tempStr[10];
  dtostrf(temp, 4, 1, tempStr);

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB12_tr);
  display.drawStr(10, 20, "Hora:");
  display.drawStr(70, 20, horaStr);
  display.drawStr(10, 45, "Temp:");
  display.drawStr(70, 45, tempStr);
  display.sendBuffer();
}

void mostrarPantalla2() {
  char buffer[6];
  sprintf(buffer, "%02d:%02d", setHora, setMin);

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB12_tr);
  display.drawStr(10, 25, "Setear Hora:");
  display.drawStr(80, 25, buffer);
  display.drawStr(0, 55, "SW1: Hora  SW2: Min");
  display.sendBuffer();
}