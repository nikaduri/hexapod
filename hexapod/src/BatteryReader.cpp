#include "BatteryReader.h"


BatteryReader::BatteryReader(int analogPin) {
    this->analogPin = analogPin;    
    pinMode(analogPin,INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(analogPin, ADC_11db);
    delay(100);
    Serial.print(analogPin);
}

BatteryReader::BatteryReader(int analogPin, adc_attenuation_t attenuation) {
    this->analogPin = analogPin;
    analogReadResolution(12);
    analogSetPinAttenuation(analogPin, attenuation);  
    delay(100);
}


float BatteryReader::getPercentage() {
    float reading = 0;
    for(int i = 0; i < samples; i++) {
        reading += analogRead(analogPin);
    }
    reading /= samples;
    Serial.println(reading);
    if(reading <= min) {
        return 0;
    } else if(reading >= max) {
        return 100;
    }
    int percentage = map(reading, min, max, 0, 100);
    Serial.println(percentage);
    return percentage;
}
