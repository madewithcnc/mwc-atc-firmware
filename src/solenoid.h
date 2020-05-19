#ifndef Solenoid_h
#define Solenoid_h

#include "Arduino.h"

class Solenoid 
{
    public:
        Solenoid(uint8_t pin);
        enum States { Off, On };
  
        void open();
        void close();
        States getState();

    private:
        uint8_t _pin;
        States _currentState;
}; 

#endif /* Solenoid_h */