// Name:    Simple IGC Logger
// Author:  Antoine Megens
// Version: 1.1
//
// Hardware Requirements:
// - Board AtMega2560 or compatible
// - Any NMEA GPS on Serial1
// - BMP280 sensor on I2C bus
// - micro SD interface
// - Lithium Ion Batterij - 3.7v 3000mAh
// - Lipo charge via micro USB
// - 3.7 to 5V DC/DC converter
//
// Libraries:
// - Arduino library for BMP280 pressure and altitude sensors. 
//   version=2.1.0
//   https://github.com/adafruit/Adafruit_BMP280_Library
//
// - Arduino SD library
//   version=1.2.4
//   http://www.arduino.cc/en/Reference/SD
//
// - TinyGPS++
//   version=1.0.0
//   https://github.com/mikalhart/TinyGPSPlus
//
// - IniFile
//   version=1.3.0
//   https://github.com/stevemarple/IniFile
//
// - DateTime - Arduino library for date and time functions
//   version=??
//   Copyright (c) Michael Margolis.  All right reserved.
//   https://www.pjrc.com/teensy/td_libs_DateTime.html
//
// - MD5 - library
//   Version=??
//   https://codebender.cc/library/MD5#MD5.cpp
//

#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <DateTime.h>
#include "utils.h"
#include "config.h"
#include "logger.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 1

//Software Serial in case you have no Serial1
//Not tested!!
//#define SOFTWARE_SERIAL
#ifdef SOFTWARE_SERIAL
#include <SoftwareDEBUG.h>
SoftwareSerial mySerial(10, 11); // RX, TX
#endif

//#define PLOT
#define NO_TONE

// Modify SD_CS_PIN for your board.
// Use SS for default CS pin (53 on AtMega2560)
#define SD_CS_PIN SS

// BMP280 sensor
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BMP280 bmp;

// GPS object
TinyGPSPlus gps;

// number of satelites from GSP
static int sats = 0;

// use USB serial as DEBUG output
#define DEBUG Serial

#ifdef PLOT
static File plotFile;
//                    12345678
static char const *plotfilename = "plot.csv";
#endif

char buffer[80];
uint8_t loop_count = 0;
bool tone_done = false;
static int start_beep;
static int duration, old_duration;
static int timer;
static bool inits = true;
static bool in_flight = false;

// configuration
config_t config;

static float batt = 0.0;
static bool bIGCFileWrite = false;
#ifdef PLOT
static bool bPlotFileWrite = false;
#endif
static unsigned long last_igc_write = 0;
static unsigned long last_plot_write = 0;
static unsigned long last_led_blink = 0;

#define LED_PIN 31 // D31

// show on LED if we have a lock or not (rate in ms)
#define NO_LOCK_BLINK_RATE  250
#define LOCK_BLINK_RATE     1000

//------------------------------------------------------------------------------
// call back for file timestamps (UTC but that's okay!)
void dateTime(uint16_t* date, uint16_t* time) {
  
 // return GPS date using FAT_DATE macro to format fields
 *date = FAT_DATE(gps.date.year(), gps.date.month(), gps.date.day());

 // return GPS time using FAT_TIME macro to format fields
 *time = FAT_TIME(gps.time.hour(), gps.time.minute(), gps.time.second());
}

void setup() 
{
    pinMode(A1, OUTPUT);            // Voltage
    pinMode(LED_PIN,OUTPUT);        // LED
    digitalWrite(LED_PIN, LOW);     // turn the LED off
    DEBUG.begin(115200);            // USB serial for debugging
    // wait until Serial (USB) port is available
    while(!DEBUG) {} // Wait

    // Initialize battery voltage measure
    // Reads the value from the specified analog pin. Arduino boards contain a multichannel, 
    // 10-bit analog to digital converter. This means that it will map input voltages
    // between 0 and the operating voltage(5V or 3.3V) into integer values between 0 and 1023.
    power_adc_enable();
    batt += (float)analogRead(A1) * (5.0/1023.0);
    DEBUG.print(F("Voltage: "));
    DEBUG.print(batt);
    DEBUG.println("V");
    power_adc_disable();
    if (batt < 3.0)
    {
      DEBUG.print(F("BATT TOO LOW!!"));
      fatal_error_blink(250);
    }

    DateTime.sync(0);   // start the clock
    Wire.begin();       // I2C for BMP280

    DEBUG.println();
    sprintf(buffer,"Simple IGC Logger V%d.%d", VERSION_MAJOR, VERSION_MINOR);
    DEBUG.println(buffer);
    sprintf(buffer,"%s %s",__DATE__, __TIME__);
    DEBUG.println(buffer);
  
    DEBUG.print(F("TinyGPSPlus version: "));
    DEBUG.println(TinyGPSPlus::libraryVersion());

//   DEBUG.print(F("Using SD Card CS pin:"));
//   DEBUG.println(SD_CS_PIN);
    DEBUG.print(F("Initializing SD card..."));
    if (!SD.begin(SD_CS_PIN)) {
      DEBUG.println(F("initialization failed!"));
      fatal_error_blink(250);
    }
    DEBUG.println(F("initialization done!"));

    // now we have SD card, read config.ini
    DEBUG.println(F("Reading config.ini..."));
    if (!readConfig("config.ini", config))
    {
      DEBUG.println(F("Error reading configuration, will run with defaults!"));
    }
    printConfig(config);
    in_flight = !config.liftoff_detection;

#ifdef SOFTWARE_SERIAL
    myDEBUG.begin(config.baudrate); // GPS
#else
    Serial1.begin(config.baudrate);  // GPS
#endif
    // init IGC logger
    IGC::initIGC();
    IGC::prepareIGCFileName();

    // BMP280 at I2C address 0x77
    while(!bmp.begin())
    {
        DEBUG.println(F("Could not find BMP280 pressure sensor!"));
        delay(1000);
    }
    DEBUG.println(F("BMP280 pressure sensor found"));
    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    DEBUG.print(F("Temperature = "));
    DEBUG.print(bmp.readTemperature());
    DEBUG.println(" °C");

    DEBUG.print(F("Pressure = "));
    DEBUG.print(bmp.readPressure()/100);
    DEBUG.println(" hPa");

    DEBUG.print(F("Approx altitude = "));
    DEBUG.print(bmp.readAltitude(SEALEVELPRESSURE_HPA)); /* MSL adjusted to standard atmosphere */
    DEBUG.println(" m (MSL)");

    // set date time callback function
    SdFile::dateTimeCallback(dateTime);
}

void loop() 
{
    static bool led_state = false;
    static unsigned long sec = 0;
    static uint8_t minu = 0;
    static uint8_t hour = 23;
    static unsigned long msec = 0;
    static unsigned long old_msec = 0;
    static float alt(NAN), old_alt;
    static float raw_deriv,derivative;
    static int16_t freq;
    static uint8_t count_lcd = 0;
    static uint8_t count_sd = 0;
    static uint8_t count_gps = 0;
    static float avg_batt;
    static uint8_t avg_batt_count = 0;
    static float ground_speed = 0.0f;
    static int elapsed;
    static long val = 0;
    
    // Update Running Time
    msec = millis();
    val = DateTime.now();
    hour = numberOfHours(val);
    minu = numberOfMinutes(val);
    sec = numberOfSeconds(val);
    
    // read aprox. altitude based on MSL standard atmosphere
    alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);

    // 1st time here?
    if (inits) 
    {
        old_alt = alt;
        old_msec = msec;
        last_igc_write = msec;
        last_plot_write = msec;
        last_led_blink = msec;
        inits = false;
    }

    // running for more than 5 seconds yet less than 10 char.
    // received from GPS?
    if (msec > 5000 && gps.charsProcessed() < 10) // uh oh
    {
      DEBUG.println("ERROR: not getting any GPS data!");
      // dump the stream to Serial
      DEBUG.println("GPS stream dump:");
      // endless loop, blinking LED will stop to show error condition, dump GPS to serial
      while (true) {
#ifndef SOFTWARE_SERIAL
          if (Serial1.available() > 0) { // any data coming in?
            DEBUG.write(Serial1.read());
#else
          if(myDEBUG.available() > 0) {
            DEBUG.write(myDEBUG.read());
#endif
        }
      }
    }
    // keep track of Vario
    elapsed = msec - old_msec;  
    // Process Derivative every ~0.1 s
    if (elapsed >= 100)
    {
        // ~ m/s
        // delta altitude divided by elapsed time
        raw_deriv = ((alt - old_alt) * 1000) /elapsed;
        old_alt = alt;
        old_msec = msec;
    }
    // 90% is old value and 10% is from raw result
    derivative = 0.9 * derivative + 0.1 * raw_deriv;
    // more than 1.5 m/s and valid GPS?
    if(!in_flight && derivative > config.liftoff_threshold && gps.location.isValid())
    {
      DEBUG.print(F("Take off! Vario="));
      DEBUG.print(derivative);
      DEBUG.print(F("m/s, Raw="));
      DEBUG.println(raw_deriv);
      in_flight = true;
    }
    
    // VARIO TONE (not used)
#define RISING_LEVEL 0.23
#define FALLING_LEVEL 0.4
#define PERIOD_MAX 400
    if (derivative >= RISING_LEVEL) 
    {
        //duration = (int) 50 + 70 / abs(derivative);
        duration = (int) ((-derivative)*70 + 400);
    }
    else if(derivative <= -FALLING_LEVEL)
    {
        //duration = (int) 100 + 200 / abs(derivative);
        duration = (int) ((derivative)*35 +  400);
    }
    else 
    {
        duration = 0;
    }
    duration = constrain(duration, 0, 400);
    freq = (int) (640 + 200 * derivative);
    if(derivative<0)freq-=250;
    freq = constrain(freq, 40, 4000);

    if (!tone_done) 
    {
        if (duration != 0)
#ifndef NO_TONE
            tone(2, freq, duration);
#endif
        tone_done = true;
        start_beep = millis();
        old_duration = duration;
    }
    timer = start_beep + 2 * old_duration - millis();
    if (timer < 10) 
    {
        tone_done = false;
    }

    // measure average battery voltage
    power_adc_enable();
    avg_batt += (float)analogRead(A1) * (5.0/1023.0);
    power_adc_disable();
    avg_batt_count ++;
    if(avg_batt_count >= 30)
    {
        avg_batt /= avg_batt_count;
        batt = avg_batt;
        avg_batt = 0;
        avg_batt_count = 0;
    }
    // LOW BAT?
    if(batt <= 3.00)
    {
        DEBUG.println(F(" BATT LOW! SHUT DOWN!"));
        DEBUG.flush();
        
        if (bIGCFileWrite)
        {
          IGC::closeIGC();
        }
#ifdef PLOT
        if (bPlotFileWrite)
        {
          plotFile.close();
        }
#endif
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        cli();
        sleep_enable();
        sei();
        sleep_mode();
    }
    
    switch (loop_count)
    {
        case 1 : // Print data
            count_lcd ++;
            if (count_lcd == 100) // limit printing
            {
                // Note: this is logger running time, not GPS time!
                sprintf(buffer,"%02d:%02d:%02d ",hour,minu,(uint8_t) sec & 0xff);
                DEBUG.print(buffer);

                // Number of GPS SATS
                DEBUG.print(F("SATS: "));
                if (gps.satellites.isValid())
                {
                  sats = gps.satellites.value();
                  sprintf(buffer,"%02d, ",sats);
                  DEBUG.print(buffer);
                }
                else
                {
                  DEBUG.print("**, ");
                }
                DEBUG.print(F("Voltage: "));
                DEBUG.print(batt);
                DEBUG.print("V, ");
                DEBUG.print(F("Altitude:      "));
                DEBUG.print(alt);
                DEBUG.print(F(" m, "));

                DEBUG.print(F("Vario:         "));
                DEBUG.print(derivative);
                DEBUG.print(F(" m/s"));
                if (gps.location.isValid())
                {
                  DEBUG.print(F(", GPS Altitude:      "));
                  DEBUG.print(gps.altitude.meters());
                  DEBUG.print(F("m, Groundspeed: "));
                  DEBUG.print(ground_speed);
                  DEBUG.print(F(" km/h"));
                }
                DEBUG.println();
                if (msec < 5000)
                {
                  // at startup dump the GPS stream to Serial for 5 sec.
                  DEBUG.println(F("GPS stream dump:"));
                  while (millis()<5000) {
#ifndef SOFTWARE_SERIAL
                    if (Serial1.available() > 0) { // any data coming in?
                      DEBUG.write(Serial1.read());
#else
                    if(myDEBUG.available() > 0) {
                      DEBUG.write(myDEBUG.read());
#endif
                    }
                  }
                  DEBUG.println();
                }
#ifdef PLOT
                if (bPlotFileWrite && msec - last_plot_write >= 1000)
                {
                  DEBUG.println(F("Writing PLOT data..."));
                  last_plot_write = msec;
                  plotFile.print(msec);
                  plotFile.print(",");
                  plotFile.print(alt);
                  plotFile.print(",");
                  plotFile.print(derivative);
                  plotFile.print(",");
                  plotFile.print(raw_deriv);
                  plotFile.print("\n");
                  plotFile.getWriteError();
                }
#endif
               count_lcd = 0;
            }
            break;
        case 2: // Read GPS & Write data to SD Card
            if (gps.location.isValid())
            {
              // get ground speed
              if (gps.speed.isValid() && gps.speed.isUpdated())
              {
                ground_speed = gps.speed.kmph();
              }

              // in flight?
              if (in_flight)
              {
                // write a "B" record to file?
                if ((msec - last_igc_write) >= (unsigned long) (config.log_interval * 1000))
                {
                  last_igc_write = msec;
                  count_sd += IGC::writeBRecord(gps, alt, config);
                }
              }
            }
            break;
        case 3:
            {
              unsigned long blink_freq = NO_LOCK_BLINK_RATE;
              if (gps.location.isValid())
              {
                blink_freq = LOCK_BLINK_RATE;
              }
              if (msec - last_led_blink > blink_freq)
              {
                digitalWrite(LED_PIN, led_state ? LOW : HIGH);    // toggle the LED
                led_state ^= 1;
                last_led_blink = msec;
              }
            }
            break;
       default:
            // reset state machine
            loop_count = 0;
            break;
    }

    // date and time known?
    if (!bIGCFileWrite && 
        gps.time.isValid() && gps.date.isValid() &&
        gps.date.month()>0 && gps.date.day()>0 &&
        gps.time.isUpdated()) 
    {
      // wait 5 loops longer
      count_gps++;
      if (count_gps > 5)
      {
        count_gps = 0;
        if (!bIGCFileWrite)
        {
            DEBUG.println(F("*******************************************"));
            DEBUG.println(F("***** GPS clock set, enable IGC write *****"));
            DEBUG.println(F("*******************************************"));
            bIGCFileWrite = IGC::createIGCFileName(gps.date.year(),gps.date.month(),gps.date.day());
            IGC::enableIGCWrite(bIGCFileWrite);
            if (!bIGCFileWrite)
            {
              DEBUG.println(F("***** ERROR enabling IGC write! *****"));
            }
            DEBUG.print(F("in_flight = "));
            DEBUG.println(in_flight);
        }
      }
    }
    loop_count++;
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/

#ifndef SOFTWARE_SERIAL
void serialEvent1(){
    static char gps_data[128];
    static int i = 0;
    char c;
    while (Serial1.available() > 0) 
    {
        // read the next char.
        c=Serial1.read();
        // and let TinyGPS++ encode it
        gps.encode(c);
        // no lock yet?
        if (!gps.location.isValid()) {
          // store data in buffer
          gps_data[i++] = c;
          // until EOL or buffer size exceeded
          if ((c == '\n' || c == '\r') || i+1 > (int) sizeof(gps_data)) {
            // close buffer
            gps_data[i++] = '\0';
            DEBUG.print(gps_data);
            i = 0;
          }
        }
    }
}
#endif
