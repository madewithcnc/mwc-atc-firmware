#ifndef ATC_Commands_h
#define ATC_Commands_h

// Commands 
//
//
namespace ATC_commands {
    static constexpr char VERSION[]             = "VE";
    static constexpr char VERSION_LONG[]        = "VL";
    
    // Solenoid Commands use Solenoid Index as start of Command
    static constexpr char TRAY_IN[]             = "1+"; 
    static constexpr char TRAY_OUT[]            = "1-"; 

    static constexpr char BLAST_ON[]            = "2+"; 
    static constexpr char BLAST_OFF[]           = "2-"; 
    // except PDB commands
    static constexpr char PDB_ACTIVATE[]        = "D+"; 
    static constexpr char PDB_DEACTIVATE[]      = "D-"; 

    static constexpr char SPINDLE_LOCK_ON[]     = "4+"; 
    static constexpr char SPINDLE_LOCK_OFF[]    = "4-"; 

    static constexpr char INDEX_TRAY[]          = "T"; 
    
    static constexpr char STATUS[]              = "ST"; 
    static constexpr char FIND_HOME[]           = "FH"; 
    static constexpr char OFFSET_UP[]           = "H+"; 
    static constexpr char OFFSET_DOWN[]         = "H-";

    static constexpr char PROFILE_IDENTIFIER[]  = "-tool";
    static constexpr char PROFILE_VFD_LEVEL[]   = "vl";
    static constexpr char PROFILE_VFD_PULSE[]   = "vp";
    static constexpr char PROFILE_VFD_BT30[]    = "bt30";
}

#endif /* ATC_Commands_h */