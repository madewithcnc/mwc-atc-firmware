#include "atc.h"

ATC _atc;

void setup() {
  _atc.init();
}

void loop() {
  _atc.processSerial();
}