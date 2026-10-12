/*
 * Detección de dirección de luz con 2 LDRs
 * Adaptado para ESP32-C6 (Sin Wi-Fi / Procesamiento Local)
 */

// Pines analógicos (ADC1) en ESP32-C6
#define LDR_LEFT_PIN  0
#define LDR_RIGHT_PIN 1

// Pines para LEDs indicadores
#define LED_LEFT_PIN  18
#define LED_RIGHT_PIN 19

// Parámetros de calibración y filtrado
#define R_FIJA 1000.0f       // Resistencia fija en Ohmios (10k)
#define MAX_ANGLE 60.0f       // Cobertura angular máxima estimado (+/- deg)
#define N_SMOOTH 8            // Tamaño de la ventana del promedio móvil
#define MIN_ADC_SUM 100       // Umbral mínimo de luz para estimación

// Buffers para suavizado (Media Móvil)
float bufferLeft[N_SMOOTH] = {0};
float bufferRight[N_SMOOTH] = {0};
int idxSmooth = 0;
bool filled = false;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Configuración de resolución ADC a 12 bits (0 - 4095)
  analogReadResolution(12);

  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(LED_RIGHT_PIN, OUTPUT);

  digitalWrite(LED_LEFT_PIN, LOW);
  digitalWrite(LED_RIGHT_PIN, LOW);

  Serial.println("\n=== Detección de Dirección de Luz (ESP32-C6) ===");
}

float readSmooth(int pin, float *buf) {
  int adc = analogRead(pin);  // Lectura 0..4095
  buf[idxSmooth] = (float)adc;

  float sum = 0;
  int count = filled ? N_SMOOTH : (idxSmooth + 1);
  for (int i = 0; i < count; ++i) {
    sum += buf[i];
  }
  return sum / (float)count;
}

void loop() {
  // Lectura y filtrado por media móvil
  float avgLeft = readSmooth(LDR_LEFT_PIN, bufferLeft);
  float avgRight = readSmooth(LDR_RIGHT_PIN, bufferRight);

  // Actualizar índice del buffer circular
  idxSmooth++;
  if (idxSmooth >= N_SMOOTH) {
    idxSmooth = 0;
    filled = true;
  }

  float sum = avgLeft + avgRight;

  // Verificación de umbral de iluminación mínima
  if (sum < MIN_ADC_SUM) {
    Serial.println("Poca luz -> Iluminación insuficiente para estimar ángulo.");
    digitalWrite(LED_LEFT_PIN, LOW);
    digitalWrite(LED_RIGHT_PIN, LOW);
    delay(200);
    return;
  }

  // Proporción de luz percibida por la derecha r en [0.0 .. 1.0]
  float r = avgRight / sum;

  // Mapeo a estimación angular diferencial (-MAX_ANGLE .. +MAX_ANGLE)
  float angle = (r - 0.5f) * 2.0f * MAX_ANGLE;

  // Umbral de activación para indicadores LED (Histeresis simple)
  const float SIDE_THRESHOLD = 0.55f; 

  if (r > SIDE_THRESHOLD) {
    digitalWrite(LED_RIGHT_PIN, HIGH);
    digitalWrite(LED_LEFT_PIN, LOW);
  } else if (r < (1.0f - SIDE_THRESHOLD)) {
    digitalWrite(LED_LEFT_PIN, HIGH);
    digitalWrite(LED_RIGHT_PIN, LOW);
  } else {
    digitalWrite(LED_LEFT_PIN, LOW);
    digitalWrite(LED_RIGHT_PIN, LOW);
  }

  // Salida por monitor serie
  Serial.print("ADC_L: ");         Serial.print(avgLeft, 1);
  Serial.print("\tADC_R: ");       Serial.print(avgRight, 1);
  Serial.print("\tr: ");           Serial.print(r, 3);
  Serial.print("\tÁngulo (deg): "); Serial.print(angle, 1);
  Serial.println();

  delay(150);
}