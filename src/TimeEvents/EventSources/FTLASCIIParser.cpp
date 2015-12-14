/**
 * format specification:
 * =====================
 * TODO
 */

#include "FTLASCIIParser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <assert.h>

#include "../../Globs/Common.hpp"
#include "../Event.hpp"


using namespace std;

FTLASCIIParser::FTLASCIIParser (string file) : TraceParser(file) {}

/**
 * Return the next event parsed from the trace file
 * Note that the event* is allocated by the parser. The application must delete it
 * after processing.
 */
Event* FTLASCIIParser::getNextEvent ()
{
  string line;

  while(_file->good())
  {
    getline(*_file, line);
    if(!mustNotParse(line))
         break;
  }

  if(!_file->good())
    return NULL;

#ifdef VERBOSE
  cout << "[PARSER] Parsing \"" << line << "\"" << endl;
#endif /* VERBOSE */

  return allocateEventFromAsciiLine(line);
}

/**
 * Return true if the full line is a comment or an empty line
 */
bool FTLASCIIParser::mustNotParse (string line)
{
  line.erase( std::remove_if( line.begin(), line.end(), ::isspace ), line.end() );

  if(!line.compare("") || line[0] == '#')
    return true;

  return false;
}

void FTLASCIIParser::print_progress ()
{
  cerr << "Print progress not implemented for FTLASCII ..." << endl;
}

/**
 * Here the allocation is performed
 */
Event* FTLASCIIParser::allocateEventFromAsciiLine(string& line)
{
  Event *res = NULL;
  double arrival_time;
  uint32_t device_num;
  uint32_t size, type;
  uint64_t addr;

  if(sscanf(line.c_str(), "%lf %u %lu %u %u", &arrival_time, &device_num,
            &addr, &size, &type) != 5)
    ERROR("Error parsing line :" + line);

  arrival_time = arrival_time * 1000;	// input file is ms, simulator need us.

  switch(type)
  {
    case 0:		// write
    {
      return new FTLSysWrite(arrival_time, addr, size);
      break;
    }

    case 1:		// read
    {
      return new FTLSysRead(arrival_time, addr, size);
      break;
    }

    case 2:		// trim (TODO how does trim work ...)
    {
      return new FTLSysTrim(arrival_time, addr, size);
      break;
    }

    default:
      ERROR("Unknown request type for :" + line);
      break;
  }

  //TODO

//  // todo repalce assert by error
//  assert(vec.size() >= 2);
//
//  // todo if there is a problem here we must indicate the line in the
//  // wl file where the problem is
//  // or maybe return NULL and the mother class takes care of error
//
//  string &syscall_type = vec[1];
//  // TODO compute arrival time here and replace below double arrival_time =
//
//  if(!syscall_type.compare("create"))
//    res = new VFSCreate(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()));
//  else if(!syscall_type.compare("truncate"))
//    res = new VFSTruncate(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()));
//  else if(!syscall_type.compare("open"))
//    res = new VFSOpen(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()), atoi(vec[4].c_str()), atoi(vec[5].c_str()));
//  else if(!syscall_type.compare("read"))
//    res = new VFSRead(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()), atoi(vec[4].c_str()));
//  else if(!syscall_type.compare("write"))
//    res = new VFSWrite(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()), atoi(vec[4].c_str()));
//  else if(!syscall_type.compare("remove"))
//    res = new VFSRemove(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()));
//  else if(!syscall_type.compare("rename"))
//    res = new VFSRename(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()));
//  else if(!syscall_type.compare("sync"))
//    res = new VFSSync(atof(vec[0].c_str())*1000);
//  else if(!syscall_type.compare("dropcache"))
//    res = new VFSDropCache(atof(vec[0].c_str())*1000);
//  else if(!syscall_type.compare("close"))
//    res = new VFSClose(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()));
//  else
//    ERROR("Unknown event type parsed :" + vec[1]);
//
//  assert(res != NULL);

  return res;
}
