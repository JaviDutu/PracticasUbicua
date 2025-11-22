// =============================================================================
// ARCHIVO: ESP32_Utils.hpp
// DESCRIPCIÓN: Funciones de utilidad genéricas para la placa ESP32.
// =============================================================================

#include <WiFi.h>

void start_wifi_connection() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Iniciando conexión a WiFi: ");
  Serial.println(ssid);
  actualizarPantalla("WiFi", "Iniciando...");
}

void handleWifi() {
  static unsigned long lastWifiCheck = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiCheck > 5000) {
    Serial.println("Reintentando conexión WiFi...");
    actualizarPantalla("WiFi", "Reintentando...");
    WiFi.disconnect();
    WiFi.reconnect();
    lastWifiCheck = millis();
  }
}