
#include <Arduino.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

// Variables para el rango
long numeroMinimo = 0;
long numeroMaximo = 0;

// Número aleatorio y número del jugador
long numeroSecreto = 0;
long intento = 0;

// Contador de intentos
int contadorIntentos = 0;

// 0: mínimo, 1: máximo, 2: jugar, 3: reiniciar
int estado = 0;

// Límite de los valores permitidos
const long LIMITE = 1000000;

// Verifica que todo el texto sea un entero válido
bool convertirNumero(String texto, long &resultado) {
  texto.trim();

  if (texto.length() == 0) {
    return false;
  }

  const char *inicio = texto.c_str();
  char *fin;
  errno = 0;

  long valor = strtol(inicio, &fin, 10);

  if (errno == ERANGE || *fin != '\0') {
    return false;
  }

  if (valor < -LIMITE || valor > LIMITE) {
    return false;
  }

  resultado = valor;
  return true;
}

// Volver al estado inicial del juego
void iniciarJuego() {
  estado = 0;
  contadorIntentos = 0;

  Serial.println();
  Serial.println("============================");
  Serial.println("    ADIVINA EL NUMERO");
  Serial.println("============================");
  Serial.println("Ingrese el numero minimo:");
}

void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);

  delay(1500);

  // Iniciar generador pseudoaleatorio
  randomSeed((uint32_t)esp_random());

  // Mostrar menú inicial
  iniciarJuego();
}

void loop() {

  // Solo procesar cuando llegan datos
  if (Serial.available() > 0) {

    // Leer texto enviado por el usuario
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    if (entrada.length() == 0) {
      return;
    }

    long numeroIngresado;

    // ESTADO 3: Reiniciar o terminar partida
    if (estado == 3) {
      if (entrada.equalsIgnoreCase("S")) {
        iniciarJuego();
      }
      else if (entrada.equalsIgnoreCase("N")) {
        Serial.println("Juego terminado.");
        Serial.println("Escriba S para jugar de nuevo.");
      }
      else {
        Serial.println("Escriba S o N:");
      }
      return;
    }

    // Validar el número ingresado
    if (!convertirNumero(entrada, numeroIngresado)) {
      Serial.println("Error: ingrese un entero valido");
      Serial.println("entre -1000000 y 1000000.");
      return;
    }

    // ESTADO 0: Número mínimo
    if (estado == 0) {

      numeroMinimo = numeroIngresado;

      Serial.print("Minimo seleccionado: ");
      Serial.println(numeroMinimo);

      Serial.println("Ingrese el numero maximo:");

      estado = 1;
    }

    // ESTADO 1: Número máximo
    else if (estado == 1) {

      if (numeroIngresado <= numeroMinimo) {
        Serial.println("Error: el maximo debe ser");
        Serial.println("mayor que el minimo.");
        Serial.println("Ingrese otro maximo:");
        return;
      }

      numeroMaximo = numeroIngresado;

      // El límite superior de random() es exclusivo
      numeroSecreto = random(numeroMinimo,
                             numeroMaximo + 1);

      contadorIntentos = 0;
      estado = 2;

      Serial.println();
      Serial.println("Numero secreto generado.");
      Serial.print("Rango: ");
      Serial.print(numeroMinimo);
      Serial.print(" a ");
      Serial.println(numeroMaximo);

      Serial.println("Adivina el numero:");
    }

    // ESTADO 2: Intentos del jugador
    else if (estado == 2) {

      intento = numeroIngresado;

      if (intento < numeroMinimo ||
          intento > numeroMaximo) {
        Serial.println("Numero fuera del rango.");
        Serial.println("Intente nuevamente:");
        return;
      }

      contadorIntentos++;

      if (intento < numeroSecreto) {

        Serial.println("El numero secreto es MAYOR.");
        Serial.println("Intenta otra vez:");

      }
      else if (intento > numeroSecreto) {

        Serial.println("El numero secreto es MENOR.");
        Serial.println("Intenta otra vez:");

      }
      else {

        Serial.println();
        Serial.println("FELICITACIONES!");
        Serial.println("Adivinaste el numero.");

        Serial.print("Numero secreto: ");
        Serial.println(numeroSecreto);

        Serial.print("Cantidad de intentos: ");
        Serial.println(contadorIntentos);

        Serial.println();
        Serial.println("Desea jugar otra vez?");
        Serial.println("S = Si / N = No");

        estado = 3;
      }
    }
  }
}

