#ifndef BATTERY_READER_H
#define BATTERY_READER_H

#include <Arduino.h>
#include "Constants.h"


class BatteryReader {
public:

    BatteryReader(int analogPin);

    float getPercentage();


private:
    const int samples = 10;
    const int min = 1452;
    const int max = 2035;
    int analogPin;
};

#endif 