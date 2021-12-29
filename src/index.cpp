#include <Arduino.h>
#include <SD.h>

void WriteNextFileIndex(int n)
{
  Serial.println(F("Opening index.txt for writing..."));
  File index = SD.open("index.txt", O_WRITE | O_CREAT | O_TRUNC);
  if (index) {
      Serial.println(F("Writing index.txt..."));
      char line[10];
      sprintf(line,"%d",n);
      index.println(line);
      index.close();
  }
  else {
    Serial.println(F("Error opening index.txt for writing!"));
  }
}

int ReadNextFileIndex()
{
  int result = 0;
  char c;
  char line[80];
  char i = 0;
  char *p = line;
  
  Serial.println(F("Opening index.txt for reading..."));
  File index = SD.open(F("index.txt"),FILE_READ);
  if (index) {
    Serial.println(F("Reading index.txt..."));
    while(index.available() && i<(int) sizeof(line)){
      c = index.read();
      *p++ = c;
      if (c == '\n') {
        *p = '\0';
      }
    }
    result = atoi(line);
    Serial.print(F("result = "));
    Serial.println(result);
    WriteNextFileIndex(++result);
  }
  else {
    Serial.println(F("Error opening file index.txt for reading, creating new file"));
    WriteNextFileIndex(0);
  }
  return result;
}
