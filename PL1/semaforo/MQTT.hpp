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
  /* --- Prueba de diagnóstico: Publicar un mensaje simple y verificar el retorno ---
  
  String mensajeSimple = "Hola desde el semaforo del Grupo 9";
  
  Serial.println("------------------------------------");
  Serial.print("Intentando publicar en el topic: ");
  Serial.println(mqtt_topic);
  Serial.print("Mensaje: ");
  Serial.println(mensajeSimple);

  // La función publish() devuelve 'true' si el mensaje se pudo poner en el buffer de salida,
  // y 'false' si falló (ej. buffer lleno, desconexión momentánea, etc.).
  bool exito = mqttClient.publish(mqtt_topic, mensajeSimple.c_str());

  if (exito) {
    Serial.println(">>> RESULTADO: ¡ÉXITO! La librería aceptó el mensaje para enviarlo.");
  } else {
    Serial.println(">>> RESULTADO: ¡FALLO! La librería NO pudo procesar el mensaje.");
  }
  Serial.println("------------------------------------");

  */
  StaticJsonDocument<512> doc;

  // 1. Datos de identidad
  doc["sensor_id"] = "TL_001_G9";
  doc["sensor_type"] = "traffic_light";
  doc["street_id"] = "ST_0246"; // Vuestro Street ID
  doc["timestamp"] = obtenerTimestamp();

  // 2. Objeto "location"
  JsonObject location = doc.createNestedObject("location");
  location["latitude"] = 40.4174738;  // Vuestra Latitud
  location["longitude"] = -3.6162871; // Vuestra Longitud

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