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

void publicarDatosMQTT() {
  StaticJsonDocument<512> doc;

  // --- Datos de identidad --- 
  doc["sensor_id"] = "TL_001";
  doc["sensor_type"] = "traffic_light";
  doc["street_id"] = "ST_1370"; 
  doc["timestamp"] = obtenerTimestamp();



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
  if (estadoActual == ESTADO_VERDE) {estadoStr = "green"; if (peticionPeaton) tiempoRestante_ms = TIEMPO_VERDE_MINIMO - tiempoTranscurrido; else tiempoRestante_ms = TIEMPO_VERDE_MAXIMO - tiempoTranscurrido;}
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
  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  Serial.println("Publicando mensaje MQTT:");
  Serial.println(buffer);
  
  bool exito = mqttClient.publish(mqtt_topic, buffer, n);
  if (exito) {
    Serial.println("ÉXITO");
    lastMqttPublishTimestamp = currentTimestamp;
    if (lastMqttPublishTimestamp - currentTimestamp > 10000) {
      Serial.println("Advertencia: Gran retraso entre publicaciones MQTT. Es posible que haya un fallo en la conexión.");
    }
  }
}