#include <Arduino.h>
#include <SD.h>
#include "logger.h"
#include "igc_file_writer.h"
#include "MD5.h"
#include "utils.h"
#include "index.h"

namespace IGC
{

static File igcFile;
//                                  012345678
static char *igcfilename = (char*) "lg000.igc";
static const int igc_id_offset = 2; // offset to numeric id
//
//                                               11111111
//                                     012345678901234567
static char igc_full_path[20];      // YYYYMMDD/lg000.igc
static bool bIGCHeaderWritten = false;
static bool bIGCFileWrite = false;
static int BRecordCount = 0;
static const char* IGC_EOL = "\r\n";

// singleton instance of igc file writer
static igc_file_writer* igc_writer_ptr = NULL;

template<size_t size>
bool IGCWriteRecord(const char(&szIn)[size]) {
    return igc_writer_ptr && igc_writer_ptr->append(szIn);
}

void initIGC()
{
  Serial.println(F(">>>>>>>>>>> initIGC <<<<<<<<<<<<<"));

  memset(&igc_full_path,0,sizeof(igc_full_path));
  bIGCHeaderWritten = false;
  BRecordCount=0;
}

void prepareIGCFileName()
{
    // read index.txt from SD for next IGC index number
    int file_index = ReadNextFileIndex();
    
    // Prepare filename with current index
    igcfilename[igc_id_offset]   = (file_index / 100) % 10 + '0';
    igcfilename[igc_id_offset+1] = (file_index / 10) % 10 + '0';
    igcfilename[igc_id_offset+2] = (file_index) % 10 + '0';
}

void DumpIGCFile(const char* path)
{
  // re-open the file for reading:
  File myFile = SD.open(path);
  if (myFile) {
    Serial.print(F("Dumping "));
    Serial.println(path);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();
      Serial.write(c);
      if (c == 0x0a)
      {
        Serial.print('\r');
      }
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening "));
    Serial.print(path);
    Serial.println(F(" for reading!"));
  }  
}

int writeIGCHeader(uint8_t y, uint8_t m, uint8_t d, config_t &config)
{
  int result = 0;
  if (!bIGCHeaderWritten)
  {
    Serial.println(F("Writing IGC Header..."));
    // create empty file
    igcFile = SD.open(igc_full_path,O_WRITE | O_CREAT | O_TRUNC);
    if(igcFile)
    {
      igcFile.close();
      BRecordCount = 0;
      //construct IGC header
      result = writeARecord(); // MUST be 1st record!
      // put in UTC time stamp and config values
      if (result) result = writeHRecord("HFDTE%02d%02d%02d",d,m,y); // DDMMYY
      if (result) result = writeHRecord("HFFXA035");
      if (result) result = writeHRecord("HFPLTPILOTINCHARGE: %s",config.pilot);
      if (result) result = writeHRecord("HFCM2CREW2: %s",config.copilot);
      if (result) result = writeHRecord("HFGTYGLIDERTYPE: %s",config.type);
      if (result) result = writeHRecord("HFGIDGLIDERID: %s",config.reg);
      if (result) result = writeHRecord("HFDTM100GPSDATUM: WGS-1984");
      if (result) result = writeHRecord("HFRHWHARDWAREVERSION: 2021");
      if (result) result = writeHRecord("HFFTYFRTYPE:Simple Arduino Logger");
      if (result) result = writeHRecord("HFGPSRECEIVER: %s",config.gps);
      if (result) result = writeHRecord("HFALGALTGPS:GEO");     // for non IGC loggers
      if (result) result = writeHRecord("HFALPALTPRESSURE:ISA");
      if (result) result = writeHRecord("HFPRSPRESSALTSENSOR: Bosch Sensortec,BMP280,max9000m");
      if (result) result = writeHRecord("HFCIDCOMPETITIONID: %s",config.cs);
      if (result) result = writeHRecord("HFCCLCOMPETITIONCLASS: %s",config.cls);
      if (result) result = writeHRecord("I023638FXA3940SIU");  // FXA and SIU number
      if (result)
      {
        bIGCHeaderWritten = true;
        // for debug!
        DumpIGCFile(igc_full_path);
      }
    }
    else 
    {
        Serial.println(F("Error opening IGC file for header!"));
    }
  }
  return result;  
}

static char* convertDoubleToDDMMmmm(double f, const char* c, pos_type type)
{
  static char result[20];
  memset(&result,0,sizeof(result));
  char tmp[10];
  
  // Abs(f)
  double d = f;
  if (d<0) d = -d;
	// degrees
	int degr = (int) d;
	double dminu = (d - degr)*60;
	int minu = floor(dminu);
	int sec = (int) .5 + (dminu - minu)*1000;
	switch (type)
	{
	case LAT:
		// DDMMmmmN/S
		sprintf(tmp, "%02d", degr);
		strcpy(result, tmp);
		sprintf(tmp, "%02d", minu);
		strcat(result, tmp);
		sprintf(tmp, "%03d", sec);
		strncat(result, tmp, 3);
		break;
	case LNG:
		// DDDMMmmmE/W
		sprintf(tmp, "%03d", degr);
		strcpy(result, tmp);
		sprintf(tmp, "%02d", minu);
		strcat(result, tmp);
		sprintf(tmp, "%03d", sec);
		strncat(result, tmp, 3);
		break;
	}
	strcat(result, c);
//  Serial.print(f,6);
//  Serial.print(F(" -> "));
//  Serial.println(result);
  return result;
}

int writeBRecord(TinyGPSPlus &gps, float alt, config_t &config)
{
    const char* c;
    igc_t cur_igc;
    char igcbuffer[100];
    int result = 0;

    // IGC file write enabled but no header written yet?
    if (bIGCFileWrite && !bIGCHeaderWritten)
    {
        IGC::writeIGCHeader(gps.date.year()-2000,gps.date.month(),gps.date.day(),config);
    }
    // prepare IGC B-record
    memset(&cur_igc,0,sizeof(cur_igc));
    memset(&cur_igc,'0',sizeof(cur_igc) - 1);
    cur_igc.a = 'A';
    cur_igc.b = 'B';
    // HHMMSS
    sprintf(igcbuffer, "%02d%02d%02d",gps.time.hour(),gps.time.minute(),gps.time.second());
    memcpy(&cur_igc.time,igcbuffer,6);
    //      12345678
    // LAT: DDMMmmmN/S
    c = gps.location.rawLat().negative ? "S" : "N";
    sprintf(igcbuffer,"%s",convertDoubleToDDMMmmm(gps.location.lat(),c, IGC::LAT));
    memcpy(&cur_igc.lat,igcbuffer,8);
    //      123456789
    // LNG: DDDMMmmmE/W
    c = gps.location.rawLng().negative ? "W" : "E";
    sprintf(igcbuffer,"%s",convertDoubleToDDMMmmm(gps.location.lng(),c, IGC::LNG));
    memcpy(&cur_igc.lng,igcbuffer,9);
    // pressure altitude in meters
    sprintf(igcbuffer,"%05d",(int) alt);
    memcpy(&cur_igc.pAlt,igcbuffer,5);
    // GPS altitude in meters
    sprintf(igcbuffer,"%05d",(int) gps.altitude.meters());
    memcpy(&cur_igc.gAlt,igcbuffer,5);

    // Using this formula to get a rough 2-sigma ehp value
    float fxa = (gps.hdop.value()/100.0) * 5.1 * 2.0;
    Serial.print("HDOP = ");
    Serial.print(gps.hdop.value()/100.0,6);
    Serial.print(", FXA = ");
    Serial.println(fxa, 6);

    sprintf(igcbuffer,"%03u", (unsigned int) fxa);
    memcpy(&cur_igc.fxa,igcbuffer,3);

    // Output the SIU (Satellites In Use) Information
    sprintf(igcbuffer,"%02u", (unsigned int) gps.satellites.value());
    memcpy(&cur_igc.siu,igcbuffer,2);

//    Serial.println(cur_igc.raw);
    result = writeRecord(cur_igc.raw, true);
    if (result)
    {
      BRecordCount++;
    }
    return result;
}

void closeIGC()
{
  igcFile.close();
}

// date as YYYY, MM, DD, obtained from GPS so UTC time
bool createIGCFileName(uint16_t y,uint16_t m, uint16_t d)
{
    char folder_name[9]; // YYYYMMDD
    memset(&folder_name,0,sizeof(folder_name));
    snprintf(folder_name,sizeof(folder_name),"%04d%02d%02d",y,m,d);
    // create the folder
    if (!SD.mkdir(folder_name))
    {
       Serial.println(F("Error creating folder on SD!"));
       return false;
    }
    // create full path name
    strcpy(igc_full_path,folder_name);
    strcat(igc_full_path,"/");
    strcat(igc_full_path,igcfilename);
    Serial.print(F("IGC path:"));
    Serial.println(igc_full_path);
    if (IGC::igc_writer_ptr == NULL)
    {
      IGC::igc_writer_ptr = new igc_file_writer(igc_full_path, true);
    }
    return true;
}

int writeRecord(const char *data, bool sign)
{
  int len = strlen(data);
  int result = 0;
  if (bIGCFileWrite)
  {
    char line[128];
    memset(&line,0,sizeof(line));
    strncpy(line,data, len);
    strcat(line, IGC_EOL);
    if (IGC::IGCWriteRecord(line))
    {
      result = 1;
    }
  }
  return result;
}

int writeARecord()
{
  // A RECORD - FR ID NUMBER. 
  // The A Record must be the first record in an FR Data File, 
  // and includes the three-character GNSS FR Serial Number (S/N) unique
  // to the manufacturer of the FR that recorded the flight.
  // Format of the A Record:
  //
  // A M M M N N N T E X T S T R I N G CR LF
  // +---------------------------------------------------------+
  // | A record Description	| Size	  | Element	| Remarks      |
  // +---------------------------------------------------------+
  // | Manufacturer	        | 3 bytes	| MMM	    | Alphanumeric |
  // | Unique ID	          | 3 bytes	| NNN	    | Alphanumeric |
  // | ID extension	        | Optional|	STRING	| Alphanumeric |
  // +---------------------------------------------------------+
  // 
  // XXX is reserved for other manufacturers, other than
  // the offical assigned three-character codes
  //
  // X and XXX are general designations for IGC format files
  // from manufacturers who do not produce an IGC-approved recorder.
  // Such recorders will not have been tested and evaluated by
  // GFAC and may not fulfil certain aspects of the IGC Specification
  // such as security protections, recording of pressure altitude, 
  // ENL or other variables. There is no guarantee that the file will
  // conform exactly to the IGC format, although specimen files will
  // be checked for compliance with the IGC format if emailed to the
  // GFAC chairman for evaluation. Even after this procedure, 
  // no compliance guarantee can be made because the type of recorder
  // will not have completed a full GFAC evaluation. It should be noted 
  // that although the file name will not contain the information, 
  // the details of the manufacturer and the recorder model concerned
  // will be identifiable (if the file conforms to the IGC standard)
  // because they will be included in the H (Header) record, 
  // see below under H Record in the line: 
  // HFFTYFRTYPE:MANUFACTURERSNAME,FRMODELNUMBER CRLF

  return writeRecord("AXLK001", true);
}

int writeHRecord(const char* format, ...)
{
  char line[80];
  va_list arg;
  va_start(arg, format);
  vsnprintf(line,sizeof(line),format,arg);
  va_end(arg);
  return writeRecord(line, true);
}

// NOTE: FILE IS EXPECTED TO BE STILL OPEN FOR WRITING!
void writeGRecord(const MD5::MD5_CTX &ctx)
{
  MD5::MD5_CTX md5_context_tmp;
  char line[80];
  char part1[17];
  char part2[17];
  memset(&line,0,sizeof(line));

  // make a copy, so we can continue with
  // next record in case flight not done
  memcpy(&md5_context_tmp,&ctx, sizeof(MD5::MD5_CTX));
  unsigned char * hash = (unsigned char *) malloc(16);
  MD5::MD5::MD5Final(hash, &md5_context_tmp);
  char *md5str = MD5::MD5::make_digest(hash,16);
  Serial.print("MD5 hash > ");
  Serial.println(md5str);
  // split in two 16 char. strings
  memset(&part1,0,sizeof(part1));
  memset(&part2,0,sizeof(part2));
  memcpy(part1,md5str,16);
  memcpy(part2,(md5str+16),16);

  snprintf(line,sizeof(line),"G%s",part1);
  igcFile.print(line);
  igcFile.print(IGC_EOL);
  if (igcFile.getWriteError()) 
  {
    Serial.println(F("Error writing G-record!"));
  }
  snprintf(line,sizeof(line),"G%s",part2);
  igcFile.print(line);
  igcFile.print(IGC_EOL);
  free(md5str);
  free(hash);
}

void enableIGCWrite(bool enable)
{
  bIGCFileWrite = enable;
}

} // IGC namespace
