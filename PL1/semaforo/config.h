// =============================================================================
// ARCHIVO: config.h
// DESCRIPCIÓN: Archivo de configuración centralizado para el proyecto.
// Contiene credenciales, pines y parámetros de comportamiento.
// =============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración de Red WiFi ---
const char* ssid = "info-uah";      
const char* password = ""; 

// --- Configuración del Broker MQTT (usamos el broker público de mosquito) ---
const char* mqtt_server = "192.168.184.165";
const int   mqtt_port = 1883;                  
const char* mqtt_user = "";
const char* mqtt_pass = "";

// --- Identidad del Dispositivo y Topics MQTT ---
const char* mqtt_client_id = "semaforo_LAB12ANA-G9"; 
const char* mqtt_topic = "sensors/ST_1370/TL_001/data"; // topic para mandar datos
const char* mqtt_sub_topic = "sensors/ST_1370/TL_001/control"; // topic para recibir datos

// --- Configuración de NTP (Servidor de Hora) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;   
const int   daylightOffset_sec = 3600;

// --- Mapeo de Pines (Hardware Abstraction Layer) ---
const int PIN_LED_ROJO = 23;
const int PIN_LED_AMARILLO = 19;
const int PIN_LED_VERDE = 18;
const int PIN_PULSADOR = 5;
const int PIN_LDR = 34;

// --- Definición global de los estados del semáforo ---
enum EstadoSemaforo { 
  ESTADO_VERDE, 
  ESTADO_AMARILLO, 
  ESTADO_ROJO,
  ESTADO_EMERGENCIA 
};

// --- Parámetros de Comportamiento del Semáforo ---
const unsigned long TIEMPO_VERDE_MINIMO = 15000;
const unsigned long TIEMPO_AMARILLO = 3000;
const unsigned long TIEMPO_ROJO = 10000;
const long intervaloPublicacion = 5000; 
// --- Umbrales brillo ---
const int BRIGHT_LEVEL = 255;
const int DIM_LEVEL = 20;
const int LDR_TRESHOLD = 400;

// --- Configuración de Pantalla OLED ---
#define ANCHO_PANTALLA 128 // Ancho en píxeles
#define ALTO_PANTALLA 64   // Alto en píxeles

#endif //CONFIG_H