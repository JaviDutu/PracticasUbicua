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

void publicarDatosMQTT() {
  StaticJsonDocument<512> doc;

  // 1. Datos de identidad
  doc["sensor_id"] = "TL_001_G9";
  doc["sensor_type"] = "traffic_light";
  doc["street_id"] = "ST_1370"; // Vuestro Street ID
  doc["timestamp"] = obtenerTimestamp();

  // 2. Crear y rellenar el objeto "location"
  JsonObject location = doc.createNestedObject("location");
  location["latitude"] = 40.4087123;
  location["longitude"] = -3.6924532;
  location["street_name"] = "Plaza del Emperador Carlos V";
  location["district"] = "Centro";
  location["neighborhood"] = "Palacio";
  location["postal_code"] = "28005";

  // 3. Objeto "data"
  JsonObject data = doc.createNestedObject("data");
  
  String estadoStr = "unknown";
  if (estadoActual == ESTADO_VERDE) estadoStr = "green";
  else if (estadoActual == ESTADO_AMARILLO) estadoStr = "yellow";
  else if (estadoActual == ESTADO_ROJO) estadoStr = "red";
  
  data["current_state"] = estadoStr;
  data["pedestrian_request"] = peticionPeaton;
  
  int lecturaLDR = analogRead(PIN_LDR);
  float porcentajeLuz = map(lecturaLDR, 200, 950, 0, 100);
  data["light_level_percent"] = constrain(porcentajeLuz, 0, 100);

  // 4. Serializar y publicar
  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  Serial.println("Publicando mensaje MQTT:");
  Serial.println(buffer);
  
  bool exito = mqttClient.publish(mqtt_topic, buffer, n);
  if (exito) {
    Serial.println("ÉXITO");
  }
}