
#include <WiFi.h>
#include <PubSubClient.h>

// CONFIGURACION WIFI
const char* WIFI_SSID = "Iotlab";
const char* WIFI_PASSWORD = "vale1556";

// IP DEL COMPUTADOR CON MOSQUITTO
const char* MQTT_BROKER = "10.193.145.39";
const int MQTT_PORT = 1883;

// TOPICOS
const char* TOPIC_POT =
  "instrumentacion/c6/potenciometro";

const char* TOPIC_LED_CMD =
  "instrumentacion/c6/led/comando";

const char* TOPIC_LED_STATE =
  "instrumentacion/c6/led/estado";

// PINES
const int PIN_POT = 0;
const int PIN_LED = 18;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long previousMillis = 0;
const unsigned long INTERVAL = 1000;
bool ledState = false;

// PUBLICAR ESTADO DEL LED
void publishLedState() {
  mqtt.publish(
    TOPIC_LED_STATE,
    ledState ? "ON" : "OFF",
    true
  );
}

// RECIBIR COMANDOS MQTT
void callback(char* topic, byte* payload,
              unsigned int length) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensaje recibido: ");
  Serial.println(message);

  if (String(topic) == TOPIC_LED_CMD) {

    if (message == "ON") {
      ledState = true;
      digitalWrite(PIN_LED, HIGH);
      Serial.println("LED ENCENDIDO");
    }
    else if (message == "OFF") {
      ledState = false;
      digitalWrite(PIN_LED, LOW);
      Serial.println("LED APAGADO");
    }
    else {
      Serial.println("Comando no valido");
      return;
    }

    publishLedState();
  }
}

// CONEXION WIFI
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());
}

// CONEXION MQTT
void connectMQTT() {

  while (!mqtt.connected()) {

    String clientId = "ESP32C6_";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str())) {

      Serial.println("MQTT conectado");

      mqtt.subscribe(TOPIC_LED_CMD);
      publishLedState();

    } else {

      Serial.print("Error MQTT: ");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

// SETUP
void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  analogReadResolution(12);

  connectWiFi();

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(callback);
}

// LOOP
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();

  unsigned long now = millis();

  if (now - previousMillis >= INTERVAL) {

    previousMillis = now;

    int value = analogRead(PIN_POT);

    char payload[12];
    snprintf(payload, sizeof(payload),
             "%d", value);

    mqtt.publish(TOPIC_POT, payload);

    Serial.print("Potenciometro: ");
    Serial.println(value);
  }
}
