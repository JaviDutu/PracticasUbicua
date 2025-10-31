// =============================================================================
// ARCHIVO: main.ino (V4.0 - Lógica de Peatón "Siempre en Verde")
// DESCRIPCIÓN: Versión final del semáforo inteligente. El sistema permanece
//              en verde hasta que un peatón pulsa el botón, momento en el que
//              inicia la secuencia para ceder el paso. Mantiene todas las
//              funcionalidades de conectividad y sensores.
// AUTORES: Javier Andrei Dutu y Konstantin Yanev Mihov (Modificado por Gemini)
// GRUPO: LAB12ANA-G9
// =============================================================================

// 1. INCLUSIÓN DE LIBRERÍAS
#include "config.h" 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// 5. DECLARACIÓN DE FUNCIONES (PROTOTIPOS)
void actualizarPantalla(String mensaje1, String mensaje2 = "");
void handleWifi();
void handleMqtt();
void publicarDatosMQTT();
void callback(char* topic, byte* message, unsigned int length);
int determinarBrilloLed();
void leerPulsador();
void actualizarLuces(bool rojo, bool amarillo, bool verde);

// 6. OBJETOS GLOBALES
Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// 7. VARIABLES GLOBALES PARA LA LÓGICA
EstadoSemaforo estadoActual = ESTADO_VERDE;
unsigned long tiempoAnteriorEstado = 0;
bool peticionPeaton = false;
int brilloActual = BRILLO_ALTO;

// Variables para control de rebotes y publicación
unsigned long ultimoTiempoRebote = 0;
const long retrasoRebote = 50;
unsigned long tiempoAnteriorPublicacion = 0;

// Incluimos el código de los ficheros .hpp al final
#include "ESP32_Utils.hpp"
#include "ESP32_Utils_MQTT.hpp"
#include "MQTT.hpp"

// =============================================================================
// SETUP: Se ejecuta una vez al inicio
// =============================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando semáforo peatonal v4.0...");

  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_AMARILLO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_PULSADOR, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);
  
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Fallo al iniciar SSD1306")); 
    while(true);
  }
  
  start_wifi_connection();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Estado inicial: Verde
  tiempoAnteriorEstado = millis();
  actualizarLuces(false, false, true); // Encender LED verde al arrancar
  actualizarPantalla("PASE", "Vehiculos");
  Serial.println("Inicialización completa. Estado: VERDE (esperando peatón)");
}

// =============================================================================
// LOOP: Se ejecuta continuamente
// =============================================================================
void loop() {
  handleWifi();
  if(WiFi.status() != WL_CONNECTED) return;

  handleMqtt();
  if(!mqttClient.connected()) return;

  // Leemos los sensores en cada ciclo
  leerPulsador();
  brilloActual = determinarBrilloLed();

  // La máquina de estados gestiona la secuencia
  switch (estadoActual) {
    case ESTADO_VERDE:
      // En estado verde, la única tarea es comprobar si un peatón ha pulsado el botón.
      if (peticionPeaton) {
        Serial.println("¡Petición de peatón recibida! Cambiando a AMARILLO.");
        estadoActual = ESTADO_AMARILLO;
        tiempoAnteriorEstado = millis(); // Reinicia el cronómetro para el estado amarillo
        
        actualizarLuces(false, true, false); // Encender LED amarillo
        actualizarPantalla("PRECAUCION", "Ceda el paso");
      }
      break;

    case ESTADO_AMARILLO:
      // Espera un tiempo fijo antes de pasar a rojo.
      if (millis() - tiempoAnteriorEstado >= TIEMPO_AMARILLO) {
        Serial.println("Cambiando a ROJO. Peatones pueden cruzar.");
        estadoActual = ESTADO_ROJO;
        tiempoAnteriorEstado = millis(); // Reinicia el cronómetro para el estado rojo
        
        actualizarLuces(true, false, false); // Encender LED rojo
        actualizarPantalla("PARE", "Crucen Peatones");
      }
      break;

    case ESTADO_ROJO:
      // Espera un tiempo fijo para que los peatones crucen.
      if (millis() - tiempoAnteriorEstado >= TIEMPO_ROJO) {
        Serial.println("Secuencia finalizada. Volviendo a VERDE.");
        estadoActual = ESTADO_VERDE;
        peticionPeaton = false; // Restablece la petición para el próximo ciclo
        tiempoAnteriorEstado = millis();
        
        actualizarLuces(false, false, true); // Encender LED verde
        actualizarPantalla("PASE", "Vehiculos");
      }
      break;
  }

  // Publicar datos por MQTT a intervalos regulares
  if (millis() - tiempoAnteriorPublicacion >= intervaloPublicacion) {
    tiempoAnteriorPublicacion = millis();
    publicarDatosMQTT();
  }
}

// =============================================================================
// FUNCIONES AUXILIARES
// =============================================================================

/**
 * @brief Lee el pulsador con lógica anti-rebotes.
 * Solo registra la petición si estamos en estado verde.
 */
void leerPulsador() {
  // Solo aceptamos nuevas peticiones si el semáforo está en verde.
  if (estadoActual == ESTADO_VERDE && digitalRead(PIN_PULSADOR) == LOW) {
    if (millis() - ultimoTiempoRebote > retrasoRebote) {
      // Si no había ya una petición, la registramos.
      if (!peticionPeaton) {
        peticionPeaton = true;
      }
      ultimoTiempoRebote = millis();
    }
  }
}

/**
 * @brief Lee el LDR y decide el nivel de brillo para los LEDs.
 */
int determinarBrilloLed() {
  int valorLDR = analogRead(PIN_LDR);
  if (valorLDR > UMBRAL_LUZ_LDR) {
    return BRILLO_ALTO;
  } else {
    return BRILLO_ATENUADO;
  }
}

/**
 * @brief Actualiza el estado de los LEDs con el brillo actual.
 */
void actualizarLuces(bool rojo, bool amarillo, bool verde) {
  analogWrite(PIN_LED_ROJO,     rojo     ? brilloActual : 0);
  analogWrite(PIN_LED_AMARILLO, amarillo ? brilloActual : 0);
  analogWrite(PIN_LED_VERDE,    verde    ? brilloActual : 0);
}

/**
 * @brief Muestra mensajes en la pantalla OLED.
 */
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
    display.setTextColor(WHITE);
    int16_t x2, y2; uint16_t w2, h2;
    display.getTextBounds(mensaje2, 0, 0, &x2, &y2, &w2, &h2);
    display.setCursor((ANCHO_PANTALLA - w2) / 2, 40);
    display.println(mensaje2);
  }
  display.display();
}

/**
 * @brief Callback para cuando se recibe un mensaje MQTT.
 */
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido en [");
  Serial.print(topic);
  Serial.print("]: ");
  String messageTemp;
  for (int i = 0; i < length; i++) { messageTemp += (char)message[i]; }
  Serial.println(messageTemp);
}