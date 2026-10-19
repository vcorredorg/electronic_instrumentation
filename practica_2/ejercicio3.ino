
#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <DHT.h>

// Pines
#define SDA_PIN 6
#define SCL_PIN 7
#define DHTPIN 4
#define DHTTYPE DHT11

// Objetos de los dispositivos
hd44780_I2Cexp lcd;
DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(115200);
  delay(1000);

  // Inicializar I2C y LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  int resultado = lcd.begin(16, 2);

  if (resultado != 0) {
    Serial.println("Error en LCD");
    while (true) {
      delay(1000);
    }
  }

  lcd.backlight();
  lcd.clear();

  // Inicializar DHT11
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("ESP32-C6");

  lcd.setCursor(0, 1);
  lcd.print("Sensor DHT11");
  delay(1500);
}

void loop() {

  float temperatura = dht.readTemperature();

  if (isnan(temperatura)) {
    Serial.println("Error leyendo DHT11");

    lcd.setCursor(0, 1);
    lcd.print("Error de lectura");
  }
  else {
    // Monitor serial
    Serial.print("Temperatura: ");
    Serial.print(temperatura, 1);
    Serial.println(" C");

    // Mostrar temperatura en LCD
    lcd.setCursor(0, 0);
    lcd.print("TEMPERATURA:");

    lcd.setCursor(0, 1);
    lcd.print("                ");

    lcd.setCursor(0, 1);
    lcd.print(temperatura, 1);
    lcd.write((uint8_t)223);  // Simbolo de grados
    lcd.print("C");
  }

  delay(2000);
}
