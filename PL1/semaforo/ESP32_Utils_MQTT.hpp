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
      
      // Asegúrate de que las variables mqtt_client_id, etc. estén en config.h
      if (mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_pass)) { 
        Serial.println("conectado!");
        actualizarPantalla("MQTT", "Conectado!");
        delay(500); // Pequeña pausa para ver el mensaje
        
        // ESTAS LÍNEAS AHORA FUNCIONARÁN CORRECTAMENTE
        tiempoAnteriorEstado = millis();
        estadoAnterior = ESTADO_VERDE; 

        // client.subscribe("topic/de/entrada");
      } else {
        Serial.print("falló, rc=");
        Serial.println(mqttClient.state());
      }
    }
  } else {
    mqttClient.loop(); // Esencial para mantener la conexión
  }
}