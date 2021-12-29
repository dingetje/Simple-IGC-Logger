#ifndef _IGC_INCLUDE_
#define _IGC_INCLUDE_

#include <Arduino.h>
#include <TinyGPS++.h>
#include "MD5.h"
#include "config.h"

namespace IGC
{
    typedef union __attribute__((__packed__))
    {
        // IGC B record
        struct __attribute__((__packed__))
        {
            char b ;        // 'B'
            char time[6];   // YYMMDD
            char lat[8];    // DDMMmmmN/S
            char lng[9];    // DDDMMmmmE/W
            char a;         // 'A'
            char pAlt[5];   // pressure altitude
            char gAlt[5];   // GPS altitude
            char fxa[3];    // 2 sigma FXA (Fix Accuracy)
            char siu[2];    // satellites in use
        };
        char raw[1+6+8+9+1+5+5+3+2+1]; // +1 for NULL terminator
    }igc_t;

    enum pos_type
    {
        LAT,
        LNG
    };

    void initIGC();
    bool createIGCFileName(uint16_t y, uint16_t m, uint16_t d);
    void closeIGC();
    void prepareIGCFileName();
    int writeRecord(const char *, bool sign=true);
    int writeIGCHeader(uint8_t y, uint8_t m, uint8_t d, config_t & config);
    bool includeRecordInGCalc(const char *in);
    int writeARecord();
    int writeBRecord(TinyGPSPlus &gps, float alt, config_t & config);
    void writeGRecord(const MD5::MD5_CTX &ctx);
    int writeHRecord(const char *format, ...);
    void enableIGCWrite(bool enable=true);
    void TestIGCLKFile(const char* path);
}

#endif