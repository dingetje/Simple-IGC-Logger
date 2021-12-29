/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   igc_file_writer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 january 2018
 */

#ifndef _LOGGER_IGC_FILE_WRITER_H_
#define _LOGGER_IGC_FILE_WRITER_H_

#include <Arduino.h>
#include "MD5.h"

// for testing!
//void copyIGC(const char* filein, const char* fileout);

class igc_file_writer final {

  igc_file_writer() = delete;
  igc_file_writer(const igc_file_writer &) = delete;
  igc_file_writer(igc_file_writer &&) = delete;

  igc_file_writer& operator=(const igc_file_writer &) = delete;
  igc_file_writer& operator=(igc_file_writer &&) = delete;

public:

  igc_file_writer(const char *file, bool grecord);

  template <size_t size> 
  bool append(const char (&data)[size]) {
    static_assert(size > 0, "invalid size");
    return append(data, size);
  }

private:
  bool append(const char *data, size_t size);

  const char *file_path; /** full path of target igc file */
  const bool add_grecord; /** true if G record must be added to file */

  long next_record_position = 0; /** position of G record */

  MD5::MD5_CTX md5_a; //= {0x63e54c01, 0x25adab89, 0x44baecfe, 0x60f25476};
  MD5::MD5_CTX md5_b; //= {0x41e24d03, 0x23b8ebea, 0x4a4bfc9e, 0x640ed89a};
  MD5::MD5_CTX md5_c; //= {0x61e54e01, 0x22cdab89, 0x48b20cfe, 0x62125476};
  MD5::MD5_CTX md5_d; //= {0xc1e84fe8, 0x21d1c28a, 0x438e1a12, 0x6c250aee};
};

#endif //_LOGGER_IGC_FILE_WRITER_H_
