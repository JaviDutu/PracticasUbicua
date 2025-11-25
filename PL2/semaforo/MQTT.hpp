// =============================================================================
// ARCHIVO: MQTT.hpp
// DESCRIPCIÓN: Funciones específicas del proyecto para la comunicación MQTT.
// =============================================================================

#include <ArduinoJson.h>
#include "time.h"

String obtenerTimestamp() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "2025-01-01T00:00:00Z"; // Devuelve un valor por defecto si falla
  }
  char timeString[25];
  strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(timeString);
}

// Variable global para almacenar el timestamp de la última publicación exitosa.
// Inicializada a Epoch Time (1970-01-01T00:00:00Z).
String lastMqttPublishTimestamp = "1970-01-01T00:00:00Z";
unsigned long time_last_mqtt_publish_ms = 0;

void publicarDatosMQTT() {
  StaticJsonDocument<512> doc;
  String currentTimestamp = obtenerTimestamp();

  // --- Datos de identidad --- 
  doc["sensor_id"] = "TL_001";
  doc["sensor_type"] = "traffic_light";
  doc["street_id"] = "ST_1370"; 
  doc["timestamp"] = currentTimestamp;

  // --- Objeto "location" ---
  JsonObject location = doc.createNestedObject("location");
  location["latitude"] = 40.4087123;
  location["longitude"] = -3.6924532;
  location["street_name"] = "Plaza del Emperador Carlos V";
  location["district"] = "Centro";
  location["neighborhood"] = "Palacio";
  location["postal_code"] = "28005";

  // --- Objeto "data" ---
  JsonObject data = doc.createNestedObject("data");
  unsigned long tiempoTranscurrido = millis() - tiempoAnteriorEstado;
  unsigned long tiempoRestante_ms = 0;

  String estadoStr = "unknown";
  if (estadoActual == ESTADO_VERDE) {estadoStr = "green"; if (peticionPeaton) tiempoRestante_ms = TIEMPO_VERDE_MINIMO - tiempoTranscurrido;}
  else if (estadoActual == ESTADO_AMARILLO) {estadoStr = "yellow"; tiempoRestante_ms = TIEMPO_AMARILLO - tiempoTranscurrido;}
  else if (estadoActual == ESTADO_ROJO) {estadoStr = "red"; tiempoRestante_ms = TIEMPO_ROJO - tiempoTranscurrido;}
  else if (estadoActual == ESTADO_EMERGENCIA) estadoStr = "emergencia";
  
  int lecturaLDR = analogRead(PIN_LDR);
  String brillo = ""; if (lecturaLDR > LDR_TRESHOLD) brillo = "Brillo_al_Maximo"; else {brillo = "Brillo_Bajo";}

  data["last_message_published"] = lastMqttPublishTimestamp;
  data["traffic_light_type"] = "mixed_vehicle_pedestrian";
  data["circulation_direction"] = "bidirectional";
  data["current_state"] = estadoStr;
  data["pedestrian_request"] = peticionPeaton;
  data["light_level_percent"] = brillo;
  data["current_state_seconds"] = tiempoTranscurrido / 1000;
  if (estadoActual != ESTADO_EMERGENCIA) data["current_state_seconds_left"] = tiempoRestante_ms / 1000;
  data["uptime_seconds"] = millis() / 1000;

  // --- Serializar y publicar ---
  char buffer[1024];
  size_t n = serializeJson(doc, buffer);

  Serial.println("Publicando mensaje MQTT:");
  Serial.println(buffer);
  
  bool exito = mqttClient.publish(mqtt_topic, buffer, n);
  if (exito) {
    Serial.println("ÉXITO");
    unsigned long currentTime_ms = millis();
    unsigned long tiempo_desde_ultima_pub = currentTime_ms - time_last_mqtt_publish_ms;
    // Si no es el primer mensaje, comprobamos cuanto tiempo ha pasado entre mensajes
    if (time_last_mqtt_publish_ms != 0) {
        unsigned long tiempo_desde_ultima_pub = currentTime_ms - time_last_mqtt_publish_ms;
        if (tiempo_desde_ultima_pub > 10000) {
           Serial.println("Advertencia: Retraso > 10 segundos entre publicaciones.");
        }
    }  
    // Actualizamos las variables globales para la *próxima* vez
    lastMqttPublishTimestamp = currentTimestamp; // El String para el JSON
    time_last_mqtt_publish_ms = currentTime_ms;       // El número para la resta
  }
}