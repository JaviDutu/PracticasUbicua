// =============================================================================
// ARCHIVO: ESP32_Utils_MQTT.hpp
// DESCRIPCIÓN: Funciones genéricas para gestionar la conexión MQTT.
// =============================================================================

#include <PubSubClient.h>
// Declaración del cliente MQTT (el objeto real vivirá en el .ino)
extern PubSubClient mqttClient;


void handleMqtt() {
  static unsigned long lastMqttReconnectAttempt = 0;
  if (!mqttClient.connected()) {
    // Intentar reconectar solo cada 5 segundos para no bloquear el loop
    if (millis() - lastMqttReconnectAttempt > 5000) {
      lastMqttReconnectAttempt = millis();
      Serial.print("Intentando conexión MQTT...");
      actualizarPantalla("MQTT", "Conectando...");
      
      // Si la conexión es exitosa, avisamos por pantalla
      if (mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_pass)) { 
        if (mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_pass)) { 
        Serial.println("Conexión con ", mqtt_server, " exitosa");
        actualizarPantalla("MQTT", "Conectado!");
        delay(1000); 
        
        // Iniciamos el circuito
        tiempoAnteriorEstado = millis();
        estadoAnterior = ESTADO_VERDE; 

        mqttClient.subscribe(mqtt_sub_topic); 
        Serial.print("Suscrito a: ");
        Serial.println(mqtt_sub_topic);
        }
      } else {
        Serial.print("falló, rc=");
        Serial.println(mqttClient.state());
      }
    }
  } else {
    mqttClient.loop(); 
  }
}