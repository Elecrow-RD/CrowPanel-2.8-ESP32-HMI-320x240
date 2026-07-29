#include <WiFi.h>

/*---------------------------------------------------------------
 * Wi-Fi credentials
 * Replace both example strings with the credentials of a 2.4 GHz network.
 *--------------------------------------------------------------*/
// Stores the access-point name used for the station connection.
const char *ssid = "elecrow888";

// Stores the matching access-point password.
const char *password = "elecrow2014";

/**
 * @brief Connect the ESP32 to Wi-Fi and print its assigned IP address.
 *
 * The Arduino runtime calls this function once after startup or reset. It
 * waits until the station is connected, so incorrect credentials keep the
 * program in the connection loop.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("connecting");
  }

  Serial.println("WiFi is connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // WiFi.disconnect();
}

/**
 * @brief Keep the station connected after the one-time setup.
 *
 * The Arduino runtime calls this function repeatedly after setup(). Automatic
 * reconnection is managed by the Wi-Fi stack.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}
