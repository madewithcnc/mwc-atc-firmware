#include <avr/pgmspace.h>
#include "atc.h"
#include "tormach/tormach-atc-commands.h"
#include "atc-responses.h"
#include "pressure-sensor.h"
#include "solenoid.h"


namespace {
    static constexpr char TERMINATOR            = '\r'; // weird line ending, *nix typically uses \n and windows \r\n
    static constexpr char NEW_LINE              = '\n';
    static constexpr uint8_t MAX_COMMAND_LENGTH = 50;

    // 770M and 1100M machines use an internal HAL pin to detect VFD motion
    // Path Pilot will tell us we are LEVEL, however internally it treats us as NONE
    enum class VDF_modes { Level, Pulse, None };
    enum class Spindle_types { TTS, BT30 };

    //Configuration
    uint8_t _maxTools = 10;
    VDF_modes _vfdMode = VDF_modes::Level;
    Spindle_types _spindleType = Spindle_types::TTS;

    // Sensors and Acuators
    static constexpr uint8_t MIN_PSI = 95;
    PressureSensor _pressureSensor;
    Solenoid _powerDrawBar(4); // Pin D4
    Solenoid _airBlast(5);  // Pin D5
}

ATC::ATC()
{
  
}

void ATC::init() 
{
  // GPIO Serial Communications
  //Serial1.begin(57600);

  // Built in Serial to USB Communications
  Serial.begin(115200);
}

void ATC::processSerial()
{
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
        procesCommand(buffer); 
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

void ATC::procesCommand(char* data)
{   
    char output_buffer[100];
    // check for status first since it is likely to be the most used command
    if (strcmp(data, Tormach_ATC_commands::STATUS) == 0) {
        // check the current PSI
        int psi = _pressureSensor.getPsi();
        int data_size = snprintf(output_buffer, sizeof output_buffer, 
                                    "P%s C%s V%s D%s\r\n", 
                                    psi < MIN_PSI ? ATC_responses::ON : ATC_responses::OFF,
                                    ATC_responses::OFF, // TODO: Sensor for tray in
                                    ATC_responses::OFF, // Path Pilot ignores our VFD sensor value
                                    _powerDrawBar.getState() == Solenoid::States::On ? ATC_responses::ON : ATC_responses::OFF);

        Serial.write(output_buffer, data_size);
    } else if (strcmp(data, Tormach_ATC_commands::PDB_ACTIVATE) == 0) {
        //Serial.print("PDB Open"); // debug statement
        _powerDrawBar.open();
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::PDB_DEACTIVATE) == 0) {
        // Serial.print("PDB Closed");  // debug statement
        _powerDrawBar.close();
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::TRAY_IN) == 0) {
        // Serial.print("Tray In"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::TRAY_OUT) == 0) {
        // Serial.print("Tray Out"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::BLAST_ON) == 0) {
        // Serial.print("Blast On"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::BLAST_OFF) == 0) {
        // Serial.print("Blast Off"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strstr(data, Tormach_ATC_commands::INDEX_TRAY) != NULL
                && (int)strstr(data, Tormach_ATC_commands::INDEX_TRAY) - (int)data == 0) {
        // Make sure the command starts with T, do not want to confuse it with somethng that has BT30 in the command                    
        int trayToIndex = atoi(data + 1);
        Serial.println(trayToIndex);

    } else if (strcmp(data, Tormach_ATC_commands::FIND_HOME) == 0) {
        // Serial.print("Home the tray"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::OFFSET_UP) == 0) {
        // Serial.print("Increment tray index up"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::OFFSET_DOWN) == 0) {
        // Serial.print("Increment tray index down"); // debug statement
        Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, Tormach_ATC_commands::VERSION) == 0 || strcmp(data, Tormach_ATC_commands::VERSION_LONG) == 0 ) {
        int data_size = snprintf(output_buffer, sizeof output_buffer, 
                                    "Not A Z-Bot Automatic Tool Changer II MMC-1.0 TOOLS:%u %s %s\r\n", 
                                    _maxTools,
                                    _vfdMode == VDF_modes::Level ? "LEVEL" : "PULSE", 
                                    _spindleType == Spindle_types::BT30 ? "BT30" : "" );

        Serial.write(output_buffer, data_size);

    } else if (strstr(data, Tormach_ATC_commands::PROFILE_IDENTIFIER) != NULL) {
        // Serial.print("New Profile"); // debug statement

        // if the profile does not have bt30 in it, default to TTS spindle type
        if (strstr(data, Tormach_ATC_commands::PROFILE_VFD_BT30) == NULL) {
            if (_spindleType != Spindle_types::TTS) {
                _spindleType = Spindle_types::TTS;
            }
        }

        char* token;
        while ((token = strsep(&data, "-"))) {
            int profileTools = atoi(token);
            if (profileTools != 0) {
                _maxTools = profileTools;

            } else if (strcmp(token, Tormach_ATC_commands::PROFILE_VFD_LEVEL) == 0) {
                if (_vfdMode != VDF_modes::Level) {
                    _vfdMode = VDF_modes::Level;
                }

            } else if (strcmp(token, Tormach_ATC_commands::PROFILE_VFD_PULSE) == 0) {
                if (_vfdMode != VDF_modes::Pulse) {
                    _vfdMode = VDF_modes::Pulse;
                }

            } else if (strcmp(token, Tormach_ATC_commands::PROFILE_VFD_BT30) == 0) {
                if (_spindleType != Spindle_types::BT30) {
                    _spindleType = Spindle_types::BT30;
                }
            }
        }

        // Path Pilot only sends a new profile string if the config does not match the machine
        //TODO: write profile to EEPROM
    } else {
        // We received a command we do not know how to process
        Serial.println(ATC_responses::UNKNOWN);
    }
}
