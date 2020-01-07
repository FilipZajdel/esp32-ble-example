# ESP32 Ble example

Example program. Uses [Kolban's library](https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils) to create tx and rx characteristics. 

Basically, when device is to smartphone/pc via BLE, the one can read/write to rx/tx characteristics. Data written to Rx characteristic can be then read from tx characteristic.
