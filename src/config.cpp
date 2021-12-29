#include "config.h"

// defaults
namespace CONFIG
{
  static const unsigned long CONFIG_DEFAULT_BAUDATE = 9600;
  static const char* CONFIG_DEFAULT_PILOT = "John Doe";
  static const char* CONFIG_DEFAULT_COPILOT = "not recorded";
  static const char* CONFIG_DEFAULT_TYPE = "Astir";
  static const char* CONFIG_DEFAULT_REG = "PH-XXX";
  static const char* CONFIG_DEFAULT_CS = "XXX";
  static const char* CONFIG_DEFAULT_CLASS = "Club";
  static const char* CONFIG_DEFAULT_GPS = "Beitian BN-880Q";
  static const float CONFIG_DEFAULT_LIFTOFF_THRESHOLD = 1.5;
  static const bool CONFIG_DEFAULT_LIFTOFF_DETECT_ENABLE = true;
  static const int CONFIG_DEFAULT_LOG_INTERVAL = 2;
}

void printErrorMessage(uint8_t e, bool eol = true)
{
  switch (e) 
  {
  case IniFile::errorNoError:
    Serial.print("no error");
    break;
  case IniFile::errorFileNotFound:
    Serial.print("file not found");
    break;
  case IniFile::errorFileNotOpen:
    Serial.print("file not open");
    break;
  case IniFile::errorBufferTooSmall:
    Serial.print("buffer too small");
    break;
  case IniFile::errorSeekError:
    Serial.print("seek error");
    break;
  case IniFile::errorSectionNotFound:
    Serial.print("section not found");
    break;
  case IniFile::errorKeyNotFound:
    Serial.print("key not found");
    break;
  case IniFile::errorEndOfFile:
    Serial.print("end of file");
    break;
  case IniFile::errorUnknownError:
    Serial.print("unknown error");
    break;
  default:
    Serial.print("unknown error value");
    break;
  }
  if (eol) 
  {
    Serial.println();
  }
}

static void getStringValue(IniFile &ini, const char *section, const char* key, char* result, const char* def_value)
{
  const size_t bufferLen = 80;
  char iniline[bufferLen];

  strcpy(result,def_value);
  if (ini.getValue(section, key, iniline, bufferLen)) 
  {
    strcpy(result,iniline);
  }
  else 
  {
    char err[200];
    snprintf(err,sizeof(err), "Could not read '%s' from section '%s', will use default '%s', error: ", 
      key,section, def_value);
    Serial.print(err);
    printErrorMessage(ini.getError());
  }
}

static void getULongValue(IniFile &ini, const char *section, const char* key, unsigned long &result, const unsigned long def_value)
{
  const size_t bufferLen = 80;
  char iniline[bufferLen];

  result = def_value;
  if (ini.getValue(section, key, iniline, bufferLen)) {
    result = atol(iniline);
  }
  else {
    char err[200];
    snprintf(err,sizeof(err),"Could not read '%s' from section '%s', will use default %ld, error: ",
        key,section,def_value);
    Serial.print(err);
    printErrorMessage(ini.getError());
  }
}

static void getIntValue(IniFile &ini, const char *section, const char* key, int &result, const int def_value)
{
  const size_t bufferLen = 80;
  char iniline[bufferLen];

  result = def_value;
  if (ini.getValue(section, key, iniline, bufferLen)) {
    result = atoi(iniline);
  }
  else {
    char err[200];
    snprintf(err,sizeof(err),"Could not read '%s' from section '%s', will use default %d, error: ",
        key,section,def_value);
    Serial.print(err);
    printErrorMessage(ini.getError());
  }
}

static void getBoolValue(IniFile &ini, const char *section, const char* key, bool &result, const bool def_value)
{
  const size_t bufferLen = 80;
  char iniline[bufferLen];

  result = def_value;
  if (ini.getValue(section, key, iniline, bufferLen)) {
    result = strcasecmp("true",iniline) == 0;
  }
  else {
    char err[200];
    snprintf(err,sizeof(err),"Could not read '%s' from section '%s', will use default %d, error: ",
        key,section,def_value);
    Serial.print(err);
    printErrorMessage(ini.getError());
  }
}

static void getDoubleValue(IniFile &ini, const char *section, const char* key, double &result, const double def_value)
{
  const size_t bufferLen = 80;
  char iniline[bufferLen];

  result = def_value;
  if (ini.getValue(section, key, iniline, bufferLen)) {
    result = atof(iniline);
  }
  else {
    char err[200];
    snprintf(err,sizeof(err),"Could not read '%s' from section '%s', will use default %f, error: ",
        key,section,def_value);
    Serial.print(err);
    printErrorMessage(ini.getError());
  }
}

/*
; Simple IGC Logger configuration file
[igcheader]
Pilot=Pietje Puk
CoPilot=John Doe
Type=Duo Discus
Registration=PH-1035
CallSign=SAL
Class=Two Seater

[gps]
; baudrate of GPS, default is 9600 Bd
Baudrate=38400
Type=Beitian BN-880Q
*/

bool readConfig(const char* iniFilename, config_t &config)
{
  const size_t bufferLen = 80;
  char iniline[bufferLen];

  // init config with defaults
  strncpy(config.pilot,CONFIG::CONFIG_DEFAULT_PILOT, sizeof(config.pilot));
  strncpy(config.copilot, CONFIG::CONFIG_DEFAULT_COPILOT, sizeof(config.copilot));
  strncpy(config.type, CONFIG::CONFIG_DEFAULT_TYPE, sizeof(config.type));
  strncpy(config.reg, CONFIG::CONFIG_DEFAULT_REG, sizeof(config.reg));
  strncpy(config.cs, CONFIG::CONFIG_DEFAULT_CS, sizeof(config.cs));
  strncpy(config.cls, CONFIG::CONFIG_DEFAULT_CLASS, sizeof(config.cls));
  config.baudrate = CONFIG::CONFIG_DEFAULT_BAUDATE;
  config.liftoff_detection = CONFIG::CONFIG_DEFAULT_LIFTOFF_DETECT_ENABLE;
  config.liftoff_threshold = CONFIG::CONFIG_DEFAULT_LIFTOFF_THRESHOLD;
  config.log_interval = CONFIG::CONFIG_DEFAULT_LOG_INTERVAL;

  IniFile ini(iniFilename);
  if (!ini.open()) 
  {
    Serial.print("Ini file '");
    Serial.print(iniFilename);
    Serial.println("' does not exist");
    return false;
  }
  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(iniline, bufferLen)) 
  {
    Serial.print("ini file ");
    Serial.print(ini.getFilename());
    Serial.print(" not valid: ");
    printErrorMessage(ini.getError());
    return false;
  }
  // read settings
  getStringValue(ini,"igcheader","Pilot", config.pilot, CONFIG::CONFIG_DEFAULT_PILOT); 
  getStringValue(ini,"igcheader","CoPilot", config.copilot, CONFIG::CONFIG_DEFAULT_COPILOT); 
  getStringValue(ini,"igcheader","Type", config.type, CONFIG::CONFIG_DEFAULT_TYPE);
  getStringValue(ini,"igcheader","CallSign", config.cs, CONFIG::CONFIG_DEFAULT_CS);
  getStringValue(ini,"igcheader","Registration", config.reg, CONFIG::CONFIG_DEFAULT_REG);
  getStringValue(ini,"igcheader","Class", config.cls, CONFIG::CONFIG_DEFAULT_CLASS);
  getStringValue(ini,"gps","Type", config.gps, CONFIG::CONFIG_DEFAULT_GPS);
  getULongValue(ini,"gps", "Baudrate", config.baudrate, CONFIG::CONFIG_DEFAULT_BAUDATE);
  getIntValue(ini,"config", "log_interval", config.log_interval, CONFIG::CONFIG_DEFAULT_LOG_INTERVAL);
  getDoubleValue(ini,"config", "liftoff_threshold", config.liftoff_threshold, CONFIG::CONFIG_DEFAULT_LIFTOFF_THRESHOLD);
  getBoolValue(ini,"config", "liftoff_detection", config.liftoff_detection, CONFIG::CONFIG_DEFAULT_LIFTOFF_DETECT_ENABLE);

  return true;
}

void printLine()
{
    Serial.println(F("======================================================"));
}

void printConfig(const config_t &config)
{
    Serial.println(F("Config:"));
    printLine();
    Serial.print(F("Pilot            : "));
    Serial.println(config.pilot);
    Serial.print(F("Co-Pilot         : "));
    Serial.println(config.copilot);
    Serial.print(F("Type             : "));
    Serial.println(config.type);
    Serial.print(F("Registration     : "));
    Serial.println(config.reg);
    Serial.print(F("Class            : "));
    Serial.println(config.cls);
    Serial.print(F("Call Sign        : "));
    Serial.println(config.cs);
    Serial.print(F("GPS Type         : "));
    Serial.println(config.gps);
    Serial.print(F("GPS Baudrate     : "));
    Serial.println(config.baudrate);
    Serial.print(F("Liftoff Detection: "));
    Serial.println(config.liftoff_detection);
    Serial.print(F("Liftoff Threshold: "));
    Serial.println(config.liftoff_threshold);
    Serial.print(F("Logger Interval  : "));
    Serial.println(config.log_interval);
    printLine();
}
