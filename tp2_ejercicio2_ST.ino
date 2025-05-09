//KIM,MORAT,GUZMAN,NISIM
#include <WiFi.h>
#include <ESP32Time.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

// Pines
#define SW1 35
#define SW2 34
#define DHTPIN 23
#define DHTTYPE DHT11

// Objetos
DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
ESP32Time rtc;

// WiFi
const char* usuario = "ORT-IoT";
const char* contra = "NuevaIOT$25";

// Estados
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

// Variables
int gmt = 0; // GMT -12 a +12
bool horaSincronizada = false; 

void setup() {
  Serial.begin(115200);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  dht.begin();
  display.begin();

  // Conexión WiFi
  WiFi.begin(usuario, contra);
  sincronizarHora();
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
        sincronizarHora();
        estado = P1;
      }
      break;
  }
}

void actualizarBotones() {
  int lecturaSW1 = digitalRead(SW1);
  int lecturaSW2 = digitalRead(SW2);

  // SW1: aumenta GMT
  switch (estadoSW1) {
    case LIBRE:
      if (lecturaSW1 == LOW && lecturaSW2 == HIGH) estadoSW1 = PULSADO;
      break;
    case PULSADO:
      if (lecturaSW1 == HIGH) {
        gmt = min(gmt+ 1, 12); 
        estadoSW1 = ESPERA_SUELTA;
      }
      break;
    case ESPERA_SUELTA:
      estadoSW1 = LIBRE;
      break;
  }

  // SW2: disminuye GMT
  switch (estadoSW2) {
    case LIBRE:
      if (lecturaSW2 == LOW && lecturaSW1 == HIGH) estadoSW2 = PULSADO;
      break;
    case PULSADO:
      if (lecturaSW2 == HIGH) {
        gmt = max(gmt - 1, -12);
        estadoSW2 = ESPERA_SUELTA;
      }
      break;
    case ESPERA_SUELTA:
      estadoSW2 = LIBRE;
      break;
  }
}

void sincronizarHora() {
  // Usa configTime para establecer GMT
  configTime(gmt * 3600, 0, "pool.ntp.org");
  struct tm timeinfo; 
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
    horaSincronizada = true;
  } else {
    Serial.println("Error al obtener hora NTP");


  }
}

void mostrarPantalla1(float temp) {
  char horaStr[9];
  sprintf(horaStr, "%02d:%02d:%02d", rtc.getHour(true), rtc.getMinute(), rtc.getSecond());

  char tempStr[10];
  dtostrf(temp, 4, 1, tempStr);

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB12_tr);
  display.drawStr(0, 20, "Hora:");
  display.drawStr(55, 20, horaStr);
  display.drawStr(10, 45, "Temp:");
  display.drawStr(70, 45, tempStr);
  display.sendBuffer();
}

void mostrarPantalla2() {
  char gmtStr[10];
  sprintf(gmtStr, "GMT %+d", gmt);

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB12_tr);
  display.drawStr(20, 25, gmtStr);
  display.drawStr(0, 55, "SW1:+  SW2:-");
  display.sendBuffer();
}
