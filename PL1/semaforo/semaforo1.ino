// =============================================================================
// ARCHIVO: main.ino (V4.0 - Lógica de Peatón "Siempre en Verde")
// DESCRIPCIÓN: Versión final del semáforo inteligente.
// El sistema permanece en verde hasta que un peatón pulsa el botón, momento en el que inicia la secuencia para ceder el paso.
// Mantiene todas las funcionalidades de conectividad y sensores.
// AUTORES: Javier Andrei Dutu y Konstantin Yanev Mihov 
// GRUPO: LAB12ANA-G9
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
void gestionarEstadoEmergencia(); 
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

// --- CAMBIO: Variables para el parpadeo de emergencia ---
unsigned long tiempoAnteriorParpadeo = 0;
const long intervaloParpadeo = 500; // 500ms encendido, 500ms apagado
bool estadoLedEmergencia = true;     // true = ON, false = OFF

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
    case ESTADO_VERDE: gestionarEstadoVerde();
break;
    case ESTADO_AMARILLO: gestionarEstadoAmarillo(); break;
    case ESTADO_ROJO: gestionarEstadoRojo(); break;
    case ESTADO_EMERGENCIA: gestionarEstadoEmergencia(); break; 
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
  
  messageTemp.toLowerCase(); 

  if (messageTemp == "peaton") {
    Serial.println("Comando 'peaton' recibido. Activando petición.");
if (!peticionPeaton) { 
      peticionPeaton = true;
      actualizarPantalla("PETICION", "RECIBIDA"); 
      delay(1000);
}
    
  } else if (messageTemp == "rojo") {
    Serial.println("Comando 'rojo' recibido. Forzando ESTADO_ROJO.");
estadoActual = ESTADO_ROJO;
    tiempoAnteriorEstado = millis(); 
    
    analogWrite(PIN_LED_VERDE, 0); 
    analogWrite(PIN_LED_AMARILLO, 0);
    
} else if (messageTemp == "accidente") { 
    Serial.println("Comando 'accidente' recibido. EMERGENCIA.");
    
    // 1. Cambiar al nuevo estado de EMERGENCIA
    estadoActual = ESTADO_EMERGENCIA;
    
    // 2. IMPORTANTE: Actualizar 'estadoAnterior' también.
    //    Esto fuerza a 'gestionarEstadoEmergencia' a 
    //    actualizar la pantalla y reiniciar el parpadeo.
//    
    
    // 3. Reiniciar el temporizador del estado
    tiempoAnteriorEstado = millis();
    
    // 4. Apagar los otros LEDs inmediatamente
analogWrite(PIN_LED_VERDE, 0); 
    analogWrite(PIN_LED_AMARILLO, 0);
    
  } else if (messageTemp == "reset") {
    Serial.println("Comando 'reset' recibido. Reiniciando ciclo a ROJO.");
    
    // 1. Forzar el estado a ROJO para reiniciar el ciclo
    estadoActual = ESTADO_ROJO;
    
    // 2. Limpiar petición de peatón
    peticionPeaton = false;
    
    // 3. Reiniciar el temporizador del estado
    tiempoAnteriorEstado = millis();
    
}
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
    analogWrite(PIN_LED_VERDE, 0);
// Apagamos el verde
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
    analogWrite(PIN_LED_AMARILLO, 0);
// Apagamos el amarillo
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
    peticionPeaton = false;
// El ciclo se completa, reseteamos la petición
    tiempoAnteriorEstado = millis();
    analogWrite(PIN_LED_ROJO, 0);
// Apagamos el rojo
  }
}

// --- CAMBIO: FUNCIÓN DE EMERGENCIA TOTALMENTE MODIFICADA ---
void gestionarEstadoEmergencia() {
  
  // 1. Actualizar la pantalla solo una vez al entrar
  if (estadoActual != estadoAnterior) {
    // Mostramos solo "ACCIDENTE" como pediste
    actualizarPantalla("ACCIDENTE"); 
    estadoAnterior = estadoActual; 
    Serial.println("Entrando en ESTADO_EMERGENCIA (PERMANENTE)");
    
    // Forzamos el inicio del parpadeo
    tiempoAnteriorParpadeo = millis(); 
    estadoLedEmergencia = true; // Empezamos con el LED encendido
  }

  // 2. Asegurarse de que los otros LEDs están apagados
  analogWrite(PIN_LED_VERDE, 0);
  analogWrite(PIN_LED_AMARILLO, 0);

  // 3. Lógica de parpadeo (Blinking logic) sin usar delay()
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnteriorParpadeo >= intervaloParpadeo) {
    
    // Ha pasado el tiempo de intervalo (500ms), reseteamos el temporizador
    tiempoAnteriorParpadeo = tiempoActual; 

    // Invertimos el estado del LED
    estadoLedEmergencia = !estadoLedEmergencia; 

    if (estadoLedEmergencia) {
      // ENCENDIDO: Usamos BRIGHT_LEVEL (255) de config.h, ignorando el LDR
      analogWrite(PIN_LED_ROJO, BRIGHT_LEVEL); 
      Serial.println("EMERGENCIA: BLINK ON");
    } else {
      // APAGADO
      analogWrite(PIN_LED_ROJO, 0); 
      Serial.println("EMERGENCIA: BLINK OFF");
    }
  }
  
  // 4. NO HAY TEMPORIZADOR DE SALIDA.
  // El sistema permanecerá en este estado hasta que
  // un mensaje MQTT "reset" lo cambie (gestionado en el callback).
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
  // En estado de emergencia, esta función se sigue llamando
  // pero 'currentBrigthness' no se usa para el LED rojo.
  int ldrValue = analogRead(PIN_LDR);
if (ldrValue > LDR_TRESHOLD) {
    currentBrigthness = BRIGHT_LEVEL;
} else {
    currentBrigthness = DIM_LEVEL;
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