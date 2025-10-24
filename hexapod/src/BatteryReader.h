#ifndef BATTERY_READER_H
#define BATTERY_READER_H

#include <Arduino.h>
#include "Constants.h"


class BatteryReader {
public:

    BatteryReader(int analogPin);
    BatteryReader(int analogPin, adc_attenuation_t attenuation);

    float getPercentage();


private:
    const int samples = 10;
    const int min = 1672; 
    const int max = 2195;
    int analogPin;
};

#endif 