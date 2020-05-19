
#include "atc.h"

static constexpr char TERMINATOR            = '\r'; // weird line ending, *nix typically uses \n and windows \r\n
static constexpr char NEW_LINE              = '\n';
static constexpr uint8_t MAX_COMMAND_LENGTH = 50;

ATC _atc;

void setup() {
  // GPIO Serial Communications
  //Serial1.begin(57600);

  // Built in Serial to USB Communications
  Serial.begin(115200);
}

void loop() {
  while (true) {
    static char buffer[MAX_COMMAND_LENGTH + 1];
    static unsigned int buffer_position = 0;

    while (Serial.available() > 0) {
      char data = Serial.read();

      switch (data) {
        // if we get a newline ignore it
        case NEW_LINE:
          break;

        case TERMINATOR:
          // terminate string  
          buffer[buffer_position] = 0;  
          // reset buffer position for next command
          buffer_position = 0; 
          _atc.procesCommand(buffer); 
          break;

        default:
          if (buffer_position < MAX_COMMAND_LENGTH) {
            buffer[buffer_position++] = data;
            break;
          }
          // we reached a max command length, must be bad data, reset
          buffer_position = 0;
          break;
      }
    }
  }
}

