
#include "sdkconfig.h"
#include <esp_log.h>

#include "ble_cb.h"

#include "BLERemoteService.h"
#include "BLEService.h"
#include "BLEAddress.h"
#include "BLEAdvertisedDevice.h"
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"
#include "BLEUUID.h"
#include "BLEClient.h"
#include "Task.h"

#include <string>
#include <functional>

extern "C" {
void app_main()
{
    const std::string name{"ESP32"};
    std::string       advertisedData{"Hey it's late"};

    BLEDevice::init(name);

    auto bleServer{BLEDevice::createServer()};
    auto bleService{bleServer->createService(
        BLEUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"))};
    auto bleAdvert{bleServer->getAdvertising()};

    auto bleTxChar{bleService->createCharacteristic(
        BLEUUID("54059634-9448-404f-9af4-7d14556f3ad8"),
        BLECharacteristic::PROPERTY_READ)};
    auto bleRxChar{bleService->createCharacteristic(
        BLEUUID("54059634-9018-404f-9af4-7d16556f3ad8"),
        BLECharacteristic::PROPERTY_WRITE)};

    bleTxChar->setCallbacks(new TxCharacteristicCb{[&](BLECharacteristic *ch) {
        ESP_LOGI(__FUNCTION__, "Reading tx characteristic");
        ch->setValue(advertisedData);
    }});
    bleRxChar->setCallbacks(new RxCharacteristicCb{[&](BLECharacteristic *ch) {
        ESP_LOGI(__FUNCTION__, "Writing to rx characteristic");
        advertisedData = ch->getValue();
    }});

    bleTxChar->setValue("read me!");

    bleService->start();
    bleAdvert->start();
}
}
