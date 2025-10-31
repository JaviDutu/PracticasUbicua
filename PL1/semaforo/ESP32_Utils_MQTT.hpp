// =============================================================================
// ARCHIVO: ESP32_Utils_MQTT.hpp (VERSIÓN FINAL CORREGIDA)
// DESCRIPCIÓN: Funciones genéricas para gestionar la conexión MQTT.
// =============================================================================

#include <PubSubClient.h>

// Declaraciones 'extern'
extern PubSubClient mqttClient;

// Ya no necesitamos acceder a las variables de estado del semáforo desde aquí,
// así que eliminamos las declaraciones 'extern' para 'estadoAnterior' y 'tiempoAnteriorEstado'
// para mantener el código limpio.

extern void actualizarPantalla(String mensaje1, String mensaje2);


void handleMqtt() {
  static unsigned long lastMqttReconnectAttempt = 0;
  if (!mqttClient.connected()) {
    if (millis() - lastMqttReconnectAttempt > 5000) {
      lastMqttReconnectAttempt = millis();
      Serial.print("Intentando conexión MQTT...");
      actualizarPantalla("MQTT", "Conectando...");
      
      if (mqttClient.connect(mqtt_client_id, mqtt_user, mqtt_pass)) { 
        Serial.println("conectado!");
        actualizarPantalla("MQTT", "Conectado!");
        delay(500); // Pequeña pausa para ver el mensaje
        
        // << LÍNEAS PROBLEMÁTICAS ELIMINADAS >>
        // La reconexión de MQTT ya no interferirá con el estado del semáforo.

        // Si necesitas suscribirte a un topic al conectar, hazlo aquí:
        // mqttClient.subscribe("tu/topic/de/entrada");

      } else {
        Serial.print("falló, rc=");
        Serial.println(mqttClient.state());
      }
    }
  } else {
    mqttClient.loop(); 
  }
}