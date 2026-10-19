
#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// =======================================
// CONFIGURACION DE PINES
// =======================================

#define SDA_PIN 6
#define SCL_PIN 7
#define POT_PIN 0

hd44780_I2Cexp lcd;

// =======================================
// VARIABLES DEL RELOJ
// =======================================

int horas = 0;
int minutos = 0;
int segundos = 0;

unsigned long tiempoAnterior = 0;
unsigned long tiempoLCD = 0;

// =======================================
// VARIABLES DEL POTENCIOMETRO
// =======================================

int valorADC = 0;
int porcentaje = 0;

// =======================================
// ACTUALIZAR RELOJ
// =======================================

void actualizarReloj() {

  unsigned long ahora = millis();

  unsigned long transcurridos =
      (ahora - tiempoAnterior) / 1000;

  if (transcurridos > 0) {

    tiempoAnterior += transcurridos * 1000UL;

    unsigned long total =
        horas * 3600UL +
        minutos * 60UL +
        segundos;

    total = (total + transcurridos) % 86400UL;

    horas = total / 3600;
    minutos = (total % 3600) / 60;
    segundos = total % 60;
  }
}

// =======================================
// LEER POTENCIOMETRO
// =======================================

void leerPotenciometro() {

  valorADC = analogRead(POT_PIN);

porcentaje = map(valorADC, 64, 3194, 0, 100);
porcentaje = constrain(porcentaje, 0, 100);
}

// =======================================
// MOSTRAR DATOS EN LCD
// =======================================

void mostrarLCD() {

  char linea1[17];
  char linea2[17];

  snprintf(linea1, sizeof(linea1),
           "HORA: %02d:%02d:%02d",
           horas, minutos, segundos);

  snprintf(linea2, sizeof(linea2),
           "POT:%4d %3d%%",
           valorADC, porcentaje);

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(linea1);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(linea2);
}

// =======================================
// AJUSTAR HORA DESDE MONITOR SERIAL
// =======================================

void ajustarHora() {

  if (Serial.available() > 0) {

    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    int h, m, s;
    char extra;

    int leidos = sscanf(
      entrada.c_str(),
      "%d:%d:%d %c",
      &h, &m, &s, &extra
    );

    if (leidos == 3 &&
        h >= 0 && h <= 23 &&
        m >= 0 && m <= 59 &&
        s >= 0 && s <= 59) {

      horas = h;
      minutos = m;
      segundos = s;

      tiempoAnterior = millis();

      Serial.println("Hora configurada.");

    } else {
      Serial.println("Formato incorrecto.");
      Serial.println("Use HH:MM:SS");
    }
  }
}

// =======================================
// SETUP
// =======================================

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(100);

  // Iniciar I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // Inicializar LCD
  int resultado = lcd.begin(16, 2);

  if (resultado != 0) {
    Serial.print("Error LCD: ");
    Serial.println(resultado);

    while (true) {
      delay(1000);
    }
  }

  lcd.backlight();

  // Configurar ADC
  analogReadResolution(12);

  // Iniciar pantalla
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("INICIANDO...");

  lcd.setCursor(0, 1);
  lcd.print("ESP32-C6 I2C");

  delay(1500);

  lcd.clear();

  tiempoAnterior = millis();
  tiempoLCD = millis();

  Serial.println("====================");
  Serial.println("RELOJ Y POTENCIOMETRO");
  Serial.println("====================");
  Serial.println("Ingrese la hora:");
  Serial.println("Formato HH:MM:SS");

  leerPotenciometro();
  mostrarLCD();
}

// =======================================
// LOOP
// =======================================

void loop() {

  // Comprobar si el usuario ajusta la hora
  ajustarHora();

  // Actualizar segundos, minutos y horas
  actualizarReloj();

  // Actualizar lectura y pantalla cada 200 ms
  if (millis() - tiempoLCD >= 200) {

    tiempoLCD = millis();

    leerPotenciometro();

    mostrarLCD();

    Serial.printf(
      "Hora: %02d:%02d:%02d | ADC: %d | POT: %d%%\n",
      horas, minutos, segundos,
      valorADC, porcentaje
    );
  }
}
