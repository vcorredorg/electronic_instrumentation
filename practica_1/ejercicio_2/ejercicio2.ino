/*
 * Monitoría: Instrumentación y Electrónica 
 * Proyecto: Conteo de Pulsos e Interrupciones (Local / Sin Wi-Fi)
 * Placa: ESP32-C6
 */

// --- Configuración de Pines ---
const int PIN_BOTON = 4; // Conectado a GND al presionar

// --- Variables para Lógica y Tiempos ---
unsigned long tiempoUltimoPulsoValido = 0; 
unsigned long diferenciaTiempo = 0;
int contadorInterrupciones = 0;

// Variables para el ISR (Debounce Diferido)
volatile bool posiblePresion = false;
volatile unsigned long tiempoInterrupcion = 0;

// --- Interrupción por Hardware (ISR) ---
void IRAM_ATTR manejadorBoton() {
  if (!posiblePresion) {
    tiempoInterrupcion = millis();
    posiblePresion = true; 
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n--- Iniciando Sistema ESP32-C6 (Conteo Local) ---");
  
  pinMode(PIN_BOTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), manejadorBoton, FALLING);
}

void loop() {
  // 1. Lógica de Debounce Diferido
  if (posiblePresion) {
    noInterrupts();
    unsigned long tInterrupcion = tiempoInterrupcion;
    interrupts();

    if (millis() - tInterrupcion > 50) {
      if (digitalRead(PIN_BOTON) == LOW) {
        unsigned long tiempoActual = millis();
        diferenciaTiempo = tiempoActual - tiempoUltimoPulsoValido;
        tiempoUltimoPulsoValido = tiempoActual;
        
        contadorInterrupciones++;
        
        // Imprimir únicamente al presionar
        Serial.print(">>> EVENTO: Hardware (Boton) | Conteo: ");
        Serial.print(contadorInterrupciones);
        Serial.print(" | Delta T: ");
        Serial.print(diferenciaTiempo);
        Serial.println(" ms <<<");
      
      }
      posiblePresion = false;
    }
  }
}
