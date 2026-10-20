
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

// ========================================
// CONFIGURACION DE PINES SPI
// ESP32-C6
// ========================================

#define BMP_SCK   6
#define BMP_MOSI  7
#define BMP_MISO  2
#define BMP_CS    10

// Crear objeto del sensor usando SPI hardware
Adafruit_BMP280 bmp(&SPI, BMP_CS);

// ========================================
// CONFIGURACION INICIAL
// ========================================

void setup() {

  Serial.begin(115200);
  delay(1500);

  Serial.println("============================");
  Serial.println("ESP32-C6 - BMP280 POR SPI");
  Serial.println("============================");

  // Inicializar SPI con los pines asignados
  SPI.begin(
    BMP_SCK,
    BMP_MISO,
    BMP_MOSI,
    BMP_CS
  );

  // Inicializar BMP280
  if (!bmp.begin()) {

    Serial.println("ERROR: BMP280 no detectado.");
    Serial.println("Revise las conexiones SPI.");

    while (true) {
      delay(1000);
    }
  }

  // Configurar el sensor
  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X1,
    Adafruit_BMP280::FILTER_X4,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println("BMP280 inicializado correctamente.");
  Serial.println("Midiendo solamente temperatura...");
  Serial.println();
}

// ========================================
// BUCLE PRINCIPAL
// ========================================

void loop() {

  // Obtener temperatura en grados Celsius
  float temperatura = bmp.readTemperature();

  // Verificar que la lectura sea valida
  if (isnan(temperatura)) {

    Serial.println("Error al leer temperatura.");

  } else {

    Serial.print("Temperatura: ");
    Serial.print(temperatura, 2);
    Serial.println(" °C");

  }

  delay(2000);
}
