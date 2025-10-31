// =============================================================================
// ARCHIVO: config.h
// DESCRIPCIÓN: Archivo de configuración centralizado para el proyecto.
// Contiene credenciales, pines y parámetros de comportamiento.
// =============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración de Red WiFi ---
const char* ssid = "javi";      
const char* password = "wifiiphone"; 

// --- Configuración del Broker MQTT (Prueba con Puerto WebSocket) ---
const char* mqtt_server = "test.mosquitto.org"; // Usamos el broker de HiveMQ
const int   mqtt_port = 1883;                  // Mantenemos el puerto estándar por ahora
const char* mqtt_user = "";
const char* mqtt_pass = "";

// --- Identidad del Dispositivo y Topics MQTT ---
const char* mqtt_client_id = "semaforo_LAB12ANA-G9-test-hive"; 
const char* mqtt_topic = "universidad/uah/cubicua/g9/semaforo"; // Un topic bien específico

// --- Configuración de NTP (Servidor de Hora) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;   // Madrid: UTC+1
const int   daylightOffset_sec = 3600;

// --- Mapeo de Pines (Hardware Abstraction Layer) ---
const int PIN_LED_ROJO = 23;
const int PIN_LED_AMARILLO = 19;
const int PIN_LED_VERDE = 18;
const int PIN_PULSADOR = 5;
const int PIN_LDR = 34;
// Los pines I2C para la pantalla (SDA=21, SCL=22) son manejados por la librería.

// AÑADE ESTO: Definición global de los estados del semáforo
enum EstadoSemaforo { 
  ESTADO_VERDE, 
  ESTADO_AMARILLO, 
  ESTADO_ROJO 
};

// --- Parámetros de Comportamiento del Semáforo ---
const unsigned long TIEMPO_VERDE_MINIMO = 10000;
const unsigned long TIEMPO_VERDE_MAXIMO = 25000;
const unsigned long TIEMPO_AMARILLO = 3000;
const unsigned long TIEMPO_ROJO = 10000;
const long intervaloPublicacion = 10000; // Publicar datos cada 15 segundos
// --- Umbrales brillo ---
const int BRIGHT_LEVEL = 255;
const int DIM_LEVEL = 20;
const int LDR_TRESHOLD = 400;

// --- Configuración de Pantalla OLED ---
#define ANCHO_PANTALLA 128 // Ancho en píxeles
#define ALTO_PANTALLA 64   // Alto en píxeles

#endif //CONFIG_H