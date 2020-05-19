#include "solenoid.h"

Solenoid::Solenoid(uint8_t pin) 
{ 
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _currentState = States::Off;
} 

void Solenoid::open() 
{ 
    digitalWrite(_pin, HIGH);
    _currentState = States::On;
} 

void Solenoid::close() 
{ 
    digitalWrite(_pin, LOW);
    _currentState = States::Off;
} 

Solenoid::States Solenoid::getState() 
{ 
    return _currentState;
} 