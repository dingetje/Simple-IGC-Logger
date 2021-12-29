#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <Arduino.h>
#include <IniFile.h>

// Config

/*
; Simple IGC Logger configuration file
[igcheader]
Pilot=Pietje Puk
CoPilot=John Doe
Glider=Duo Discus
Registration=PH-1035
CallSign=SAL
Class=Two Seater

[gps]
; baudrate of GPS, default is 9600 Bd
Baudrate=38400
Type=Beitian BN-880Q

[config]
liftoff_detection=true
liftoff_threshold=1.5
log_interval=2
*/

typedef struct __attribute__((__packed__))
{
    char pilot[80];
    char copilot[80];
    char type[40];
    char reg[10];
    char cs[10];
    char cls[20];
    char gps[50];
    unsigned long baudrate;
    bool liftoff_detection;
    double liftoff_threshold;
    int log_interval;
} config_t;

bool readConfig(const char* iniFilename, config_t &config);
void printConfig(const config_t &config);

#endif