#include <Arduino.h>
#include <SD.h>
#include "utils.h"

void fatal_error_blink(const int d)
{
      // hangup, with fast blinking LED
      while(1) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(d);
        digitalWrite(LED_BUILTIN, LOW);
        delay(d);
      };
}
