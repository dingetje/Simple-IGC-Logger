/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   igc_file_writer.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 january 2018
 * 
 * Ported to arduino by Antoine Megens, 15-Feb-2021
 */

#include <Arduino.h>
#include <MD5.h>
#include <SD.h>
#include "igc_file_writer.h"
#include "utils.h"

namespace {
  // return c if valid char for IGC files
  // return space if not.
  char clean_igc_char(char c) {
    if (c >= 0x20 && c <= 0x7E && c != 0x24 &&
        c != 0x2A && c != 0x2C && c != 0x21 &&
        c != 0x5C && c != 0x5E && c != 0x7E) {
      return c;
    }
    return ' ';
  }

  void write_g_record(File &stream, const MD5::MD5_CTX &md5) {
    char part1[17];
    char part2[17];
    MD5::MD5_CTX md5_tmp;
    // we made copy to allow to continue to update hash after Final call.
  // make a copy, so we can continue with
  // next record in case flight not done
  memcpy(&md5_tmp,&md5, sizeof(MD5::MD5_CTX));
  unsigned char * hash = (unsigned char *) malloc(16);
  MD5::MD5::MD5Final(hash, &md5_tmp);
  char *md5str = MD5::MD5::make_digest(hash,16);
//  Serial.print("MD5 hash > ");
//  Serial.println(md5str);
  // split in two 16 char. strings
  memset(&part1,0,sizeof(part1));
  memset(&part2,0,sizeof(part2));
  memcpy(part1,md5str,16);
  memcpy(part2,(md5str+16),16);

  stream.print("G");
  stream.print(part1);
  stream.print("\r\nG");
  if (stream.getWriteError()) 
  {
    Serial.println(F("Error writing G-record!"));
  }
  stream.print(part2);
  stream.print("\r\n");
  if (stream.getWriteError()) 
  {
    Serial.println(F("Error writing G-record!"));
  }
  free(md5str);
  free(hash);
  }
} // namespace

igc_file_writer::igc_file_writer(const char *file, bool grecord)
    : file_path(file), add_grecord(grecord) {
        // LK8000, not yet working, for a test file OK though!
  MD5::MD5::MD5Initialize(&md5_a, (unsigned long) 0x63e54c01, (unsigned long) 0x25adab89, (unsigned long) 0x44baecfe, (unsigned long) 0x60f25476);
  MD5::MD5::MD5Initialize(&md5_b, (unsigned long) 0x41e24d03, (unsigned long) 0x23b8ebea, (unsigned long) 0x4a4bfc9e, (unsigned long) 0x640ed89a);
  MD5::MD5::MD5Initialize(&md5_c, (unsigned long) 0x61e54e01, (unsigned long) 0x22cdab89, (unsigned long) 0x48b20cfe, (unsigned long) 0x62125476);
  MD5::MD5::MD5Initialize(&md5_d, (unsigned long) 0xc1e84fe8, (unsigned long) 0x21d1c28a, (unsigned long) 0x438e1a12, (unsigned long) 0x6c250aee);
}


bool igc_file_writer::append(const char *data, size_t size) {

  uint8_t mode = O_WRITE;
  if (next_record_position <= 0)
  {
    // if a G-record is written, we'll use seek
    // to reposition file write location
    // and do not want to use O_APPEND
    // since this breaks the seek function!!
    // (must be an arduino thing...)
    mode |= O_APPEND;
  }
  File igcFile = SD.open(file_path,mode);
  if(igcFile) 
  {
      if (next_record_position > 0)
      {
        if (!igcFile.seek(next_record_position))
        {
          Serial.println(F("Seek failed!!"));
        }
      }

    char c;
    for (; *(data) && size > 1; ++data, --size) {
      if ((*data) != 0x0D && (*data) != 0x0A) {
        c = clean_igc_char(*data);

        if (add_grecord) {
          MD5::MD5::MD5Update(&md5_a,&c,1);
          MD5::MD5::MD5Update(&md5_b,&c,1);
          MD5::MD5::MD5Update(&md5_c,&c,1);
          MD5::MD5::MD5Update(&md5_d,&c,1);
        }
      } else {
        c = *data;
      }
      igcFile.print(c);
    }

    next_record_position = igcFile.position();

    if (add_grecord) {
      write_g_record(igcFile, md5_a);
      write_g_record(igcFile, md5_b);
      write_g_record(igcFile, md5_c);
      write_g_record(igcFile, md5_d);
    }
    igcFile.close();
    return true;
  }
  return false;
}
