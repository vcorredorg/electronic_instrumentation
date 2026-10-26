#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- DEFINICIÓN DE PINES ---
const int pinSensor = 4;    // Pin digital conectado a OUT1 del sensor infrarrojo
const int pinLED = LED_BUILTIN;       // Pin del LED (integrado o externo en el pin 2)

// Variables globales de BLE (¡Aquí declaramos pService correctamente!)
BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// UUIDs estándar para simular un puerto UART por BLE
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// --- VARIABLES PARA EL CONTROL DE TIEMPO (NO BLOQUEANTE) ---
unsigned long previousMillis = 0;
const long interval = 1000; // Intervalo de 1 segundo (1000 ms) para enviar datos del sensor

// Callbacks para gestionar la conexión del celular
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Callbacks para recibir comandos desde el celular (Control del LED del pin 2)
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("Comando recibido vía BLE: ");
        Serial.println(rxValue.c_str());

        // Lógica para controlar el LED físico según el comando recibido
        if (rxValue.indexOf("ON") >= 0) {
          digitalWrite(pinLED, HIGH);
          Serial.println("-> LED Encendido");
        } else if (rxValue.indexOf("OFF") >= 0) {
          digitalWrite(pinLED, LOW);
          Serial.println("-> LED Apagado");
        }
      }
    }
};

void setup() {
  Serial.begin(115200);

  // Configuración de pines
  pinMode(pinSensor, INPUT);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW); // Iniciar apagado

  // 1. Inicializar el dispositivo BLE con el nombre solicitado
  BLEDevice::init("ESP32C6_Aguilera");

  // 2. Crear el servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3. Crear el servicio BLE
  pService = pServer->createService(SERVICE_UUID);

  // 4. Crear la característica TX (Para enviar datos del sensor al celular)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // 5. Crear la característica RX (Para recibir comandos de control de LED)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID_RX,
                                        BLECharacteristic::PROPERTY_WRITE
                                      );
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Iniciar el servicio
  pService->start();

  // Iniciar publicidad para que el celular encuentre el ESP32
  pServer->getAdvertising()->start();
  Serial.println("Dispositivo BLE 'ESP32C6_Aguilera' listo y esperando conexión...");
}

void loop() {
  // --- TAREA NO BLOQUEANTE: Lectura y envío periódico del sensor ---
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Actualizamos el temporizador

    // Leer el sensor infrarrojo digital
    int estadoSensor = digitalRead(pinSensor);
    
    String mensajeSensor = "";
    if (estadoSensor == LOW) {
      mensajeSensor = "Obstaculo Detectado\n";
    } else {
      mensajeSensor = "Zona Libre\n";
    }

    // Si hay un dispositivo conectado, enviar las notificaciones BLE
    if (deviceConnected) {
      pTxCharacteristic->setValue(mensajeSensor.c_str());
      pTxCharacteristic->notify();
      Serial.print("Enviando por BLE (No bloqueante): ");
      Serial.print(mensajeSensor);
    }
  }

  // --- GESTIÓN DE RECONEXIÓN ---
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Pequeña pausa de seguridad solo al desconectarse
    pServer->startAdvertising(); // Reiniciar publicidad
    Serial.println("Dispositivo desconectado. Reiniciando publicidad...");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("¡Dispositivo móvil conectado exitosamente!");
  }
}