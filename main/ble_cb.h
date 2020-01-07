#pragma once

#include <functional>
#include "BLECharacteristic.h"

class RxCharacteristicCb : public BLECharacteristicCallbacks {
  public:
    RxCharacteristicCb(std::function<void(BLECharacteristic *)> onRead)
        : cbonRead(onRead)
    {
    }

  private:
    void onWrite(BLECharacteristic *characteristic)
    {
        cbonRead(characteristic);
    }
    std::function<void(BLECharacteristic *)> cbonRead;
};

class TxCharacteristicCb : public BLECharacteristicCallbacks {
  public:
    TxCharacteristicCb(std::function<void(BLECharacteristic *)> onRead)
        : cbonRead(onRead)
    {
    }

  private:
    void onRead(BLECharacteristic *characteristic)
    {
        cbonRead(characteristic);
    }
    std::function<void(BLECharacteristic *)> cbonRead;
};