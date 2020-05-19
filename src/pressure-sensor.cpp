#include "pressure-sensor.h"

uint16_t PressureSensor::getPsi() 
{ 
    // analogRead is a blocking call, taking aproximately 110 µs
    uint16_t sensorValue = analogRead(_pin);

    // add 0.5 to the ADC value to account for the AVR approximation error
    // that is also why we are using 1024 instead of 1023 for our ADC divisor
    float sensorVoltage = ((float)sensorValue + 0.5) * _adcUnitsPerVolt;

    return (sensorVoltage - _minVoltage) * (_maxPressure - _minPressure) / (_maxVoltage - _minVoltage) + _minPressure;
} 