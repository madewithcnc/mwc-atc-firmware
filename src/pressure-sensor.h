#ifndef Pressure_Sensor_h
#define Pressure_Sensor_h

#include "Arduino.h"

/*
  Pressure Transducer 
  Operating Voltage: 5v
  Input: 0 - 150 psi
  Accuracy: +/- 0.5%
  Output: 0.5v - 4.5v
  Output Equation: y = x * (4 / 150) + 0.5
    Samples:  80 psi = 2.633v
              90 psi = 2.9v
             100 psi = 3.167v
             110 psi = 3.433v
             120 psi = 3.7v
             130 psi = 3.967v
             140 psi = 4.233v

  Implementation Notes: The voltage at the max expected operating range is 3.7v if more precision is needed
                        then using a 4.096 precision ADC voltage reference is more than sufficient, but would require changing the
                        variables below
*/
class PressureSensor 
{
    private:
        static constexpr float _vRef = 5.0;
        static constexpr float _adcUnitsPerVolt = _vRef / 1024.0;
        static constexpr uint8_t _pin = 23;

        // Sensor properties
        static constexpr float _minVoltage = 0.5;
        static constexpr float _maxVoltage = 4.5;
        static constexpr float _minPressure = 0.0;
        static constexpr float _maxPressure = 150.0;
    
    public: 
        uint16_t getPsi();
}; 

#endif /* Pressure_Sensor_h */