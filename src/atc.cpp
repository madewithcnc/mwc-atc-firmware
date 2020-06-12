#include <avr/pgmspace.h>
#include "atc.h"
#include "tormach/tormach-atc-commands.h"
#include "tormach/tormach-atc-responses.h"
#include "pressure-sensor.h"
#include "solenoid.h"

namespace ATC_commands = Tormach_ATC_commands;
namespace ATC_responses = Tormach_ATC_responses;


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

// Use an internal reference to Serial
// This makes it easier to switch between software serial for dev and hardware serial
auto ATC_Serial = Serial;

ATC::ATC()
{

}

void ATC::init() 
{
  // GPIO Serial Communications
  //Serial1.begin(57600);

  // Built in Serial to USB Communications
  ATC_Serial.begin(115200);
}

void ATC::processSerial()
{
  static char buffer[MAX_COMMAND_LENGTH + 1];
  static unsigned int buffer_position = 0;

  while (ATC_Serial.available() > 0) {
    char data = ATC_Serial.read();

    switch (data) {
      // if we get a newline ignore it
      case NEW_LINE:
        break;

      case TERMINATOR:
        // terminate string  
        buffer[buffer_position] = 0;  
        // reset buffer position for next command
        buffer_position = 0; 
        processCommand(buffer); 
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

void ATC::processCommand(char* data)
{   
    char output_buffer[100];
    // check for status first since it is likely to be the most used command
    if (strcmp(data, ATC_commands::STATUS) == 0) {
        // check the current PSI
        int psi = _pressureSensor.getPsi();
        int data_size = snprintf(output_buffer, sizeof output_buffer, 
                                    "P%s C%s V%s D%s\r\n", 
                                    psi < MIN_PSI ? Tormach_ATC_responses::ON : ATC_responses::OFF,
                                    ATC_responses::OFF, // TODO: Sensor for tray in
                                    ATC_responses::OFF, // Path Pilot ignores our VFD sensor value
                                    _powerDrawBar.getState() == Solenoid::States::On ? ATC_responses::ON : ATC_responses::OFF);

        ATC_Serial.write(output_buffer, data_size);
    } else if (strcmp(data, ATC_commands::PDB_ACTIVATE) == 0) {
        //ATC_Serial.print("PDB Open"); // debug statement
        _powerDrawBar.open();
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::PDB_DEACTIVATE) == 0) {
        // ATC_Serial.print("PDB Closed");  // debug statement
        _powerDrawBar.close();
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::TRAY_IN) == 0) {
        // ATC_Serial.print("Tray In"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::TRAY_OUT) == 0) {
        // ATC_Serial.print("Tray Out"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::BLAST_ON) == 0) {
        // ATC_Serial.print("Blast On"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::BLAST_OFF) == 0) {
        // ATC_Serial.print("Blast Off"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strstr(data, ATC_commands::INDEX_TRAY) != NULL
                && (int)strstr(data, ATC_commands::INDEX_TRAY) - (int)data == 0) {
        // Make sure the command starts with T, do not want to confuse it with somethng that has BT30 in the command                    
        int trayToIndex = atoi(data + 1);
        ATC_Serial.println(trayToIndex);

    } else if (strcmp(data, ATC_commands::FIND_HOME) == 0) {
        // ATC_Serial.print("Home the tray"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::OFFSET_UP) == 0) {
        // ATC_Serial.print("Increment tray index up"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::OFFSET_DOWN) == 0) {
        // ATC_Serial.print("Increment tray index down"); // debug statement
        ATC_Serial.println(ATC_responses::OK); 

    } else if (strcmp(data, ATC_commands::VERSION) == 0 || strcmp(data, ATC_commands::VERSION_LONG) == 0 ) {
        int data_size = snprintf(output_buffer, sizeof output_buffer, 
                                    "Not A Z-Bot Automatic Tool Changer II MWC-1.0 TOOLS:%u %s %s\r\n", 
                                    _maxTools,
                                    _vfdMode == VDF_modes::Level ? "LEVEL" : "PULSE", 
                                    _spindleType == Spindle_types::BT30 ? "BT30" : "" );

        ATC_Serial.write(output_buffer, data_size);

    } else if (strstr(data, ATC_commands::PROFILE_IDENTIFIER) != NULL) {
        // ATC_Serial.print("New Profile"); // debug statement

        // if the profile does not have bt30 in it, default to TTS spindle type
        if (strstr(data, ATC_commands::PROFILE_VFD_BT30) == NULL) {
            if (_spindleType != Spindle_types::TTS) {
                _spindleType = Spindle_types::TTS;
            }
        }

        char* token;
        while ((token = strsep(&data, "-"))) {
            int profileTools = atoi(token);
            if (profileTools != 0) {
                _maxTools = profileTools;

            } else if (strcmp(token, ATC_commands::PROFILE_VFD_LEVEL) == 0) {
                if (_vfdMode != VDF_modes::Level) {
                    _vfdMode = VDF_modes::Level;
                }

            } else if (strcmp(token, ATC_commands::PROFILE_VFD_PULSE) == 0) {
                if (_vfdMode != VDF_modes::Pulse) {
                    _vfdMode = VDF_modes::Pulse;
                }

            } else if (strcmp(token, ATC_commands::PROFILE_VFD_BT30) == 0) {
                if (_spindleType != Spindle_types::BT30) {
                    _spindleType = Spindle_types::BT30;
                }
            }
        }

        // Path Pilot only sends a new profile string if the config does not match the machine
        //TODO: write profile to EEPROM
    } else {
        // We received a command we do not know how to process
        ATC_Serial.println(ATC_responses::UNKNOWN);
    }
}
