#include "BatteryReader.h"


BatteryReader::BatteryReader(int analogPin) {
    this->analogPin = analogPin;
    pinMode(analogPin, INPUT);
}


float BatteryReader::getPercentage() {
    float reading = 0;
    for(int i = 0; i < samples; i++) {
        reading += analogRead(analogPin);
    }
    reading /= samples;
    if(reading <= min) {
        return 0;
    } else if(reading >= max) {
        return 100;
    }
    int percentage = map(reading, min, max, 0, 100);
    return percentage;
}