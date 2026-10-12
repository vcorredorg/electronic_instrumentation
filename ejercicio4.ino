#include <Arduino.h>
#include <math.h>

//                   PINES
// XIAO ESP32-C6
const int LDR_IZQ = A0;        // D0 / GPIO0
const int LDR_DER = A1;        // D1 / GPIO1

// LED externo
const int LED_PWM = D3;        // D3 / GPIO21

//              PARÁMETROS DEL CIRCUITO
// Alimentación de los divisores de tensión
const float VCC = 3.3;

// Resistencia fija usada con cada LDR
const float R_FIJA = 10000.0;   // 10 kΩ

//                  CONFIGURACIÓN PWM
const int PWM_FREQ = 5000;     // 5 kHz
const int PWM_RES  = 8;        // 8 bits

// Con 8 bits:
// 0   -> 0 %
// 255 -> 100 %
const int PWM_MAX = 255;

//                 SENSIBILIDAD DEL SISTEMA
const float GANANCIA = 4.0;

// Zona muerta para evitar que pequeñas variaciones
// o ruido hagan parpadear el LED.
const float ZONA_MUERTA = 0.02;


// Corrección perceptual del brillo.
const float GAMMA = 2.0;

//                 PROMEDIO DEL ADC
// Número de muestras que promediamos.
const int NUM_MUESTRAS = 10;

//       FUNCIÓN PARA LEER EL ADC PROMEDIADO
float leerVoltajePromedio(int pin) {

  uint32_t suma_mV = 0;

  for (int i = 0; i < NUM_MUESTRAS; i++) {

    suma_mV += analogReadMilliVolts(pin);

    delay(2);
  }

  float promedio_mV =
      suma_mV / (float)NUM_MUESTRAS;

  // Convertimos milivoltios a voltios
  return promedio_mV / 1000.0;
}

//          CALCULAR RESISTENCIA DEL LDR
// La ecuación del divisor es:
// Vadc = Vcc * Rfija / (Rldr + Rfija)
//
// Despejando:
// Rldr = Rfija * (Vcc/Vadc - 1)
float calcularResistencia(float voltaje) {

  // Evitamos división entre cero
  if (voltaje < 0.01) {
    voltaje = 0.01;
  }

  float Rldr =
      R_FIJA * ((VCC / voltaje) - 1.0);

  // Por errores del ADC podría salir ligeramente
  // negativo si el voltaje está muy cerca de 3.3 V.
  if (Rldr < 0) {
    Rldr = 0;
  }

  return Rldr;
}

//                       SETUP
void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("Sistema LDR + PWM iniciado");
  Serial.println();

  // ADC de 12 bits:
  // 0 - 4095
  analogReadResolution(12);

  // Configuración del PWM
  bool pwmOK =
      ledcAttach(
        LED_PWM,
        PWM_FREQ,
        PWM_RES
      );

  if (!pwmOK) {

    Serial.println(
      "ERROR: no se pudo configurar PWM"
    );
  }


  // LED inicialmente apagado
  ledcWrite(LED_PWM, 0);
}

//                       LOOP
void loop() {
  // 1. LEER LOS DOS LDR
  float V_izq =
      leerVoltajePromedio(LDR_IZQ);

  float V_der =
      leerVoltajePromedio(LDR_DER);

  // 2. CALCULAR RESISTENCIAS
  float R_izq =
      calcularResistencia(V_izq);

  float R_der =
      calcularResistencia(V_der);

  // 3. CALCULAR DIFERENCIA RELATIVA
  // -1  -> mayor luz hacia un lado
  //  0  -> iluminación equilibrada
  // +1  -> mayor luz hacia el otro lado
  float denominador =
      R_izq + R_der;

  float contraste = 0.0;


  if (denominador > 1.0) {

    contraste =
      (R_der - R_izq)
      /
      denominador;
  }

  // 4. AMPLIFICAR EL CONTRASTE
  contraste =
      contraste * GANANCIA;
  // Limitar entre -1 y +1
  contraste =
      constrain(
        contraste,
        -1.0,
        1.0
      );

  // 5. ZONA MUERTA
  if (fabs(contraste) < ZONA_MUERTA) {

    contraste = 0.0;
  }

  // 6. CONVERTIR A NIVEL DE BRILLO
  // contraste = -1 -> nivel = 0
  // contraste =  0 -> nivel = 0.5
  // contraste = +1 -> nivel = 1
  float nivel =
      (contraste + 1.0) / 2.0;
  nivel =
      constrain(
        nivel,
        0.0,
        1.0
      );

  // 7. CORRECCIÓN PERCEPTUAL
  float nivelCorregido =
      pow(nivel, GAMMA);

  // 8. NIVEL -> PWM
  int pwm =
      (int)(
        nivelCorregido
        *
        PWM_MAX
      );

  pwm =
      constrain(
        pwm,
        0,
        PWM_MAX
      );

  // 9. GENERAR PWM
  ledcWrite(
    LED_PWM,
    pwm
  );

  // 10. CALCULAR DUTY CYCLE
  float duty =
      (
        pwm
        /
        255.0
      )
      *
      100.0;

  // 11. MOSTRAR POR SERIAL
  Serial.print("R_izq:");
  Serial.print(R_izq, 0);

  Serial.print("\t");

  Serial.print("R_der:");
  Serial.print(R_der, 0);

  Serial.print("\t");

  Serial.print("Duty:");
  Serial.println(duty, 1);

  delay(500);
}