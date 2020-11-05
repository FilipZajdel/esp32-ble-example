
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
#include "PWM.h"
#include "GPIO.h"

#include <string>
#include <array>
#include <sstream>
#include <iostream>
#include <cstdlib>

/* temporary */
#define RED_LED_GPIO 26
#define GREEN_LED_GPIO 12
#define BLUE_LED_GPIO 14

#define BUTTON0_GPIO GPIO_NUM_25
#define BUTTON1_GPIO GPIO_NUM_27

constexpr unsigned defineColor(uint8_t red, uint8_t green, uint8_t blue)
{
    return (red << 16 | green << 8 | blue);
}

class RgbLed
{
  public:
    RgbLed(unsigned redGpio, unsigned greenGpio, unsigned blueGpio);
    enum Colors : unsigned {
        BLUE       = defineColor(0, 0, 255),
        RED        = defineColor(255, 0, 0),
        GREEN      = defineColor(0, 255, 0),
        MAGENTA    = defineColor(255, 0, 255),
        CYAN       = defineColor(0, 255, 255),
        AQUAMARINE = defineColor(127, 255, 212),
        ORANGE     = defineColor(255, 69, 0),
        WHITE      = defineColor(255, 255, 255),
    };

    void     setColor(Colors color);
    void     setColor(std::string colorName);
    void     setBrightness(unsigned percent);
    unsigned getBrightness() const;
    void     off();
    void     on();

  private:
    std::array<PWM, 3>            rgbLeds;
    bool                          isOn;
    std::map<std::string, Colors> colorNames;
    Colors                        currentColor;
    std::string                   colorToName(const Colors color);
    unsigned                      brightness;
};

extern "C" {
void app_main()
{
    const std::string name{"Colorful Lamp"};
    RgbLed            rgbLed{RED_LED_GPIO, GREEN_LED_GPIO, BLUE_LED_GPIO};
    BLEDevice::init(name);

    auto bleServer{BLEDevice::createServer()};
    auto bleService{bleServer->createService(
        BLEUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"))};
    auto bleAdvert{bleServer->getAdvertising()};

    auto colorControl{bleService->createCharacteristic(
        BLEUUID("54059634-9448-404f-9af4-7d14556f3ad8"),
        BLECharacteristic::PROPERTY_WRITE)};
    auto onOffControl{bleService->createCharacteristic(
        BLEUUID("54059634-9018-404f-9af4-7d16556f3ad9"),
        BLECharacteristic::PROPERTY_WRITE)};
    auto brightnessControl{bleService->createCharacteristic(
        BLEUUID("54059634-9448-404f-9af4-7d14556f3ada"),
        BLECharacteristic::PROPERTY_WRITE)};

    colorControl->setCallbacks(
        new RxCharacteristicCb{[&](BLECharacteristic *ch) {
            std::string color = ch->getValue();
            ESP_LOGI(__FUNCTION__, "Setting color %s", color.c_str());
            rgbLed.setColor(color);
        }});

    onOffControl->setCallbacks(
        new RxCharacteristicCb{[&](BLECharacteristic *ch) {
            auto value = ch->getValue();
            ESP_LOGI(__FUNCTION__, "Turning led %s", ch->getValue().c_str());

            if (value == std::string("1")) {
                rgbLed.on();
                ESP_LOGI(__FUNCTION__, "Turning on led");
            } else if (value == std::string("0")) {
                ESP_LOGI(__FUNCTION__, "Turning off led");
                rgbLed.off();
            }
        }});

    brightnessControl->setCallbacks(
        new RxCharacteristicCb{[&](BLECharacteristic *ch) {
            unsigned           brightness;
            std::istringstream ss(ch->getValue());
            ss >> brightness;

            ESP_LOGI(__FUNCTION__, "Setting brightness %s (%u)",
                     ch->getValue().c_str(), brightness);

            rgbLed.setBrightness(brightness);
        }});

    colorControl->setValue("Color (color name)");
    onOffControl->setValue("Toggle (1/0)");
    brightnessControl->setValue("Brightness (%)");

    rgbLed.on();

    bleService->start();
    bleAdvert->start();

    while (1) {
        Task::delay(5000);
    }
}
}

RgbLed::RgbLed(unsigned red, unsigned green, unsigned blue)
    : rgbLeds({PWM(RED_LED_GPIO), PWM(GREEN_LED_GPIO, LEDC_CHANNEL_1),
               PWM(BLUE_LED_GPIO, LEDC_CHANNEL_2)}),
      colorNames({
          {"white", RgbLed::Colors::WHITE},
          {"orange", RgbLed::Colors::ORANGE},
          {"red", RgbLed::Colors::RED},
          {"blue", RgbLed::Colors::BLUE},
          {"green", RgbLed::Colors::GREEN},
          {"magenta", RgbLed::Colors::MAGENTA},
          {"cyan", RgbLed::Colors::CYAN},
          {"aquamarine", RgbLed::Colors::AQUAMARINE},
      }),
      currentColor(RgbLed::Colors::WHITE), brightness(8)
{
    off();
}

void RgbLed::setColor(RgbLed::Colors color)
{
    auto red = (color >> 16) & 0xFF, green = (color >> 8) & 0xFF,
         blue = color & 0xFF;

    if (!isOn) {
        return;
    }

    rgbLeds[0].setDutyPercentage((red / 255.0) * 100);
    rgbLeds[1].setDutyPercentage((green / 255.0) * 100);
    rgbLeds[2].setDutyPercentage((blue / 255.0) * 100);

    this->currentColor = color;
    ESP_LOGI(__FUNCTION__, "Current color is %s", colorToName(color).c_str());
}

void RgbLed::setBrightness(unsigned percent)
{
    auto red = (currentColor >> 16) & 0xFF, green = (currentColor >> 8) & 0xFF,
         blue   = currentColor & 0xFF;
    float ratio = percent / 100.0;

    if (!isOn) {
        return;
    }

    if (percent > 100) {
        ratio = 100.0;
    }

    rgbLeds[0].setDutyPercentage(red * ratio);
    rgbLeds[1].setDutyPercentage(green * ratio);
    rgbLeds[2].setDutyPercentage(blue * ratio);

    brightness = percent;
}

void RgbLed::on()
{
    isOn = true;
    setColor(currentColor);
    setBrightness(brightness);
}

void RgbLed::off()
{
    for (auto &led : rgbLeds) {
        led.setDuty(0);
    }
    isOn = false;
}

void RgbLed::setColor(std::string newColor)
{
    for (auto colorName : colorNames) {
        if (colorName.first == newColor) {
            setColor(colorName.second);
        }
    }
}

std::string RgbLed::colorToName(const Colors color)
{
    for (auto colorName : colorNames) {
        if (colorName.second == color) {
            return colorName.first;
        }
    }

    return std::string{""};
}

unsigned RgbLed::getBrightness() const
{
    return brightness;
}