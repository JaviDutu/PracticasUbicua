// =============================================================================
// ARCHIVO: main.ino (V2.2 - Lógica de Botón y Timeout Mejorada)
// =============================================================================

#include "config.h" 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- Prototipos de Funciones ---
void actualizarPantalla(String mensaje1, String mensaje2 = "");
void checkLightLevel();
void handleWifi();
void handleMqtt();
void publicarDatosMQTT();
void callback(char* topic, byte* message, unsigned int length);
// --- Objetos Globales ---
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
// --- Variables Globales de Lógica ---
EstadoSemaforo estadoActual;
EstadoSemaforo estadoAnterior = ESTADO_ROJO; 
unsigned long tiempoAnteriorEstado = 0;
bool peticionPeaton = false;
int currentBrigthness;
unsigned long ultimoTiempoRebote = 0;
const long retrasoRebote = 50;
unsigned long tiempoAnteriorPublicacion = 0;

#include "ESP32_Utils.hpp"
#include "ESP32_Utils_MQTT.hpp"
#include "MQTT.hpp"

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_PULSADOR, INPUT);
  
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Fallo al iniciar SSD1306")); while(true);
  }
  
  start_wifi_connection();

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  estadoActual = ESTADO_ROJO;
  tiempoAnteriorEstado = millis();
  Serial.println("Inicialización completa. Loop principal iniciado.");
}

// =============================================================================
// LOOP
// =============================================================================
void loop() {
  handleWifi();
  if(WiFi.status() != WL_CONNECTED) return;

  handleMqtt();
  if(!mqttClient.connected()) return;
  leerPulsador();
  checkLightLevel(); // <--- Esta función actualiza currentBrigthness en cada ciclo

  switch (estadoActual) {
    case ESTADO_VERDE: gestionarEstadoVerde(); break;
    case ESTADO_AMARILLO: gestionarEstadoAmarillo(); break;
    case ESTADO_ROJO: gestionarEstadoRojo(); break;
  }

  if (millis() - tiempoAnteriorPublicacion >= intervaloPublicacion) {
    tiempoAnteriorPublicacion = millis();
    publicarDatosMQTT();
  }
}

// =============================================================================
// FUNCIONES DE LÓGICA Y CONTROL
// =============================================================================

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido en [");
  Serial.print(topic);
  Serial.print("]: ");
  String messageTemp;
  for (int i = 0; i < length; i++) { messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);
}


// --- Función de Estado Verde MEJORADA ---
void gestionarEstadoVerde() {
  if (estadoActual != estadoAnterior) {
    actualizarPantalla("PASE");
    estadoAnterior = estadoActual;
    Serial.println("Entrando en ESTADO_VERDE");
  }

  // <-- Mantenemos el brillo actualizado en tiempo real
  analogWrite(PIN_LED_VERDE, currentBrigthness);

  unsigned long tiempoTranscurrido = millis() - tiempoAnteriorEstado;
  // Condición 1: Petición de peatón Y ha pasado el tiempo mínimo
  bool transicionPorPeaton = peticionPeaton && (tiempoTranscurrido >= TIEMPO_VERDE_MINIMO);
  // Condición 2: Ha pasado el tiempo máximo (timeout de seguridad)
  bool transicionPorTimeout = tiempoTranscurrido >= TIEMPO_VERDE_MAXIMO;
  // Si CUALQUIERA de las dos condiciones es verdadera, cambiamos de estado
  if (transicionPorPeaton || transicionPorTimeout) {
    if (transicionPorPeaton) {
      Serial.println("Transición por peatón + tiempo mínimo cumplido.");
      peticionPeaton = false;
    }
    if (transicionPorTimeout) {
      Serial.println("Transición por timeout de seguridad (25s).");
    }
    
    estadoActual = ESTADO_AMARILLO;
    tiempoAnteriorEstado = millis();
    analogWrite(PIN_LED_VERDE, 0); // Apagamos el verde
  }
}

void gestionarEstadoAmarillo() {
  if (estadoActual != estadoAnterior) {
    actualizarPantalla("PRECAUCION");
    estadoAnterior = estadoActual;
    Serial.println("Entrando en ESTADO_AMARILLO");
  }
  
  // <-- Mantenemos el brillo actualizado en tiempo real
  analogWrite(PIN_LED_AMARILLO, currentBrigthness);

  if (millis() - tiempoAnteriorEstado >= TIEMPO_AMARILLO) {
    estadoActual = ESTADO_ROJO;
    tiempoAnteriorEstado = millis();
    analogWrite(PIN_LED_AMARILLO, 0); // Apagamos el amarillo
  }
}

void gestionarEstadoRojo() {
  if (estadoActual != estadoAnterior) {
    actualizarPantalla("PARE");
    estadoAnterior = estadoActual;
    Serial.println("Entrando en ESTADO_ROJO");
  }

  // <-- Mantenemos el brillo actualizado en tiempo real
  analogWrite(PIN_LED_ROJO, currentBrigthness);

  if (millis() - tiempoAnteriorEstado >= TIEMPO_ROJO) {
    estadoActual = ESTADO_VERDE;
    peticionPeaton = false; // El ciclo se completa, reseteamos la petición
    tiempoAnteriorEstado = millis();
    analogWrite(PIN_LED_ROJO, 0); // Apagamos el rojo
  }
}

// --- Función de lectura de pulsador CON MÁS DEPURACIÓN ---
void leerPulsador() {
  if (digitalRead(PIN_PULSADOR) == HIGH && (millis() - ultimoTiempoRebote > retrasoRebote)) {
    if (!peticionPeaton) {
      peticionPeaton = true;
      Serial.println("Petición de peatón registrada.");
    }
    ultimoTiempoRebote = millis();
  }
}

void checkLightLevel() {
  int ldrValue = analogRead(PIN_LDR);
  Serial.println(ldrValue);
  if (ldrValue > LDR_TRESHOLD) {
    currentBrigthness = BRIGHT_LEVEL;
    Serial.println("Alto");
  } else {
    currentBrigthness = DIM_LEVEL;
    Serial.println("Bajo");
  }
}

// ESTA ES LA FUNCIÓN CORREGIDA
// La definición (el código) ya NO lleva el argumento por defecto.
void actualizarPantalla(String mensaje1, String mensaje2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  int16_t x1, y1; uint16_t w1, h1;
  display.getTextBounds(mensaje1, 0, 0, &x1, &y1, &w1, &h1);
  display.setCursor((ANCHO_PANTALLA - w1) / 2, 10);
  display.println(mensaje1);
  if (mensaje2 != "") {
    display.setTextSize(1);
    int16_t x2, y2; uint16_t w2, h2;
    display.getTextBounds(mensaje2, 0, 0, &x2, &y2, &w2, &h2);
    display.setCursor((ANCHO_PANTALLA - w2) / 2, 40);
    display.println(mensaje2);
  }
  display.display();
}