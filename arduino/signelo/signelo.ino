/*
  Signelo : Bluetooth controlled Bike Light (BLE)
  By µsini (LABSud) - Rémi Sarrailh
  https://github.com/usini/signelo
  Licence : MIT

  Credits:
  - Ported to Arduino ESP32 by Evandro Copercini
  - Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
  - Modification by Juan Antonio Villalpando. : http://kio4.com/arduino/160i_Wemos_ESP32_BLE.htm
*/
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define NAME "Signelo"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/* Neopixels Animation (WS2812FX) */
#include <WS2812FX.h>
#define LED_COUNT 32 // Number of LEDS
#define LED_PIN 16 // Leds Pins
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/* BLE Control */
class BLEControl: public BLECharacteristicCallbacks {

    // When a message is sent to the ESP32
    void onWrite(BLECharacteristic *pCharacteristic) {
      String state = pCharacteristic->getValue().c_str();
      Serial.println(state);

      if (state == "gauche") {
        /* FX on 0 to 15 and Black to 16 to 31 */
        ws2812fx.setSegment(0, 0, 15, FX_MODE_COMET, RED, 500, false);
        ws2812fx.setSegment(1, 16, 31,     FX_MODE_STATIC, BLACK, 1000, false);
      }
      if (state == "droite") {
        /* FX on 16 to 31 and Black to 0 to 15 */
        ws2812fx.setSegment(0, 0, 15, FX_MODE_STATIC, BLACK, 1000, false);
        ws2812fx.setSegment(1, 16, 31,     FX_MODE_COMET, RED, 500, true);
      }
    }
};

/* Callback for Connection / Disconnection */
class ConnectionManager: public BLEServerCallbacks {
    void onConnect(BLEServer* MyServer) {
      // Show we are connected by turning off light
      ws2812fx.setMode(FX_MODE_STATIC);
      ws2812fx.setColor(BLACK);
      Serial.println("Connected");
    };

    void onDisconnect(BLEServer* MyServer) {
      // Smartphone has disconnect, we restart to get ready for a new connection
      Serial.println("Disconnected");
      ESP.restart();
    }
};

void setup() {
  // Lower CPU Frequency (save ~ 30ma)
  setCpuFrequencyMhz(80);

  Serial.begin(115200);

  // Initialize LED
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(200);
  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  ws2812fx.start();

  // Initialize Bluetooth
  BLEDevice::init(NAME); // Name
  BLEServer *pServer = BLEDevice::createServer(); // Create a BLE Server
  pServer->setCallbacks(new ConnectionManager()); // Connection Callbacks

  // Create service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create Characteristics
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  // Create callbacks
  pCharacteristic->setCallbacks(new BLEControl());
  pCharacteristic->setValue("Init");
  pService->start();

  // Advertise Bluetooth Service
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  // Manage Neopixels animation
  ws2812fx.service();
}
