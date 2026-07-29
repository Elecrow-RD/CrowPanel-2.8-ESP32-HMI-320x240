#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <BLECharacteristic.h>

/*---------------------------------------------------------------
 * BLE service configuration
 * The name is advertised to nearby clients, while the UUIDs identify the
 * service and its read/write/notify characteristic.
 *--------------------------------------------------------------*/
#define bleServerName "ESP32SPI-BLE"
#define SERVICE_UUID "6479571c-2e6d-4b34-abe9-c35116712345"
#define CHARACTERISTIC_UUID "826f072d-f87c-4ae6-a416-6ffdcaa02d73"

// Points to the advertising controller created by the BLE stack.
BLEAdvertising* pAdvertising = NULL;

// Points to the server that accepts BLE client connections.
BLEServer* pServer = NULL;

// Points to the application service hosted by the server.
BLEService *pService = NULL;

// Points to the value that clients can read, write, or subscribe to.
BLECharacteristic* pCharacteristic = NULL;

// Records whether a BLE client currently has an active connection.
bool connected_state = false;

/*---------------------------------------------------------------
 * BLE connection callbacks
 * The BLE stack invokes these methods when a client connects or disconnects.
 *--------------------------------------------------------------*/
class MyServerCallbacks: public BLEServerCallbacks {
  /**
   * @brief Record that a client has connected to the server.
   *
   * The BLE stack calls this callback after a connection is established.
   *
   * @param pServer Server that accepted the connection.
   * @return Nothing.
   */
  void onConnect(BLEServer *pServer) {
    connected_state = true;
  }

  /**
   * @brief Record that the active client has disconnected.
   *
   * The BLE stack calls this callback when the connection ends.
   *
   * @param pServer Server from which the client disconnected.
   * @return Nothing.
   */
  void onDisconnect(BLEServer *pServer) {
    connected_state = false;
  }
};

/**
 * @brief Create and advertise the BLE service and characteristic.
 *
 * The Arduino runtime calls this function once after startup or reset. The
 * characteristic initially contains the text ELECROW and supports read,
 * write, and notification operations.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);

  BLEDevice::init(bleServerName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setValue("ELECROW");

  /*---------------------------------------------------------------
   * Start BLE advertising
   * Publishing the service UUID lets a scanner identify this example.
   *--------------------------------------------------------------*/
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  pService->start();

  // pAdvertising->stop();
  // pService->stop();
}

/**
 * @brief Leave connection handling to the BLE stack.
 *
 * The Arduino runtime calls this function repeatedly after setup(). BLE events
 * are delivered through MyServerCallbacks, so no polling is required here.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}
