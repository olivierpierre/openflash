/**
 * format specification:
 * =====================
 * <time (milliseconds)>;<syscall>;<param1>;<param2>;etc.
 *
 * create takes 2 parameters : the inode number of the file being created
 *  and the lenght of the name of the file (both integers)
 *
 * truncate takes also 2 parameters : the inode of the truncated file and
 *  the new size of the file (both integers)
 *
 * open takes 4 parameters : inode (integer), a boolean indicating if the
 *  file must be truncated to size zero, another boolean indicating that the
 *  file must be created if it does not exists, and a last parameter indicating
 *  the name length (integer) of the file in case it is created
 *
 * read and write take 3 integer parameters : inode, address to read / write and
 *  size to read / write
 *
 * remove take one parameter (integer) which is the inode to delete
 *
 * rename takes 2 parameters (integers) : inode and new file name length
 *
 * sync takes no parameters
 */

#include "FFSCSVParser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <assert.h>

#include "../../Globs/Common.hpp"
#include "../../Globs/Simulation.hpp"

using namespace std;

FFSCSVParser::FFSCSVParser (string file) : TraceParser(file), _current_line(0)
{
  _total_lines = 0;
  std::string line;
  std::ifstream myfile(file.c_str());

  while (std::getline(myfile, line))
      _total_lines++;
}

/**
 * Return the next event parsed from the trace file
 * Note that the event* is allocated by the parser. The application must delete it
 * after processing.
 */
Event* FFSCSVParser::getNextEvent ()
{
  string line;

  while(_file->good())
  {
    getline(*_file, line);
    _current_line++;
    if(!mustNotParse(line))
         break;
  }

  if(!_file->good())
    return NULL;

#ifdef VERBOSE
  cout << "[PARSER] Parsing \"" << line << "\"" << endl;
#endif /* VERBOSE */

  if(_print_progress)
    print_progress();

  vector<string> fields = breakCsvLine(line);
  return allocateEventFromStringVector(fields);
}

/**
 * Return true if the full line is a comment or an empty line
 */
bool FFSCSVParser::mustNotParse (string line)
{
  line.erase( std::remove_if( line.begin(), line.end(), ::isspace ), line.end() );

  if(!line.compare("") || line[0] == '#')
    return true;

  return false;
}

/**
 * Break one line of the trace file into a vector of string, each one
 * being a field of the line
 */
vector<string> FFSCSVParser::breakCsvLine (string line, char sep)
{
  stringstream strstr(line);
  string cell;
  vector<string> res;

  while(getline(strstr, cell, sep))
  {
   cell.erase( std::remove_if( cell.begin(), cell.end(), ::isspace ), cell.end() );
   res.push_back(cell);
  }

  return res;
}

void FFSCSVParser::print_progress ()
{
	cerr << _current_line << " / " << _total_lines << endl;
}

/**
 * Here the allocation is performed
 */
Event* FFSCSVParser::allocateEventFromStringVector (vector<string>& vec)
{
  Event *res = NULL;

  if(vec.size() < 2)
    ERROR("Unknown format on line " + nts(_current_line) + " of trace file");
  assert(vec.size() >= 2);

  // todo if there is a problem here we must indicate the line in the
  // wl file where the problem is
  // or maybe return NULL and the mother class takes care of error

  string &syscall_type = vec[1];
  // TODO compute arrival time here and replace below double arrival_time =

  if(!syscall_type.compare("create"))
    res = new VFSCreate(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()));
  else if(!syscall_type.compare("truncate"))
    res = new VFSTruncate(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()));
  else if(!syscall_type.compare("open"))
    res = new VFSOpen(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()), atoi(vec[4].c_str()), atoi(vec[5].c_str()));
  else if(!syscall_type.compare("read"))
    res = new VFSRead(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()), atoi(vec[4].c_str()));
  else if(!syscall_type.compare("write"))
    res = new VFSWrite(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()), atoi(vec[4].c_str()));
  else if(!syscall_type.compare("remove"))
    res = new VFSRemove(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()));
  else if(!syscall_type.compare("rename"))
    res = new VFSRename(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()), atoi(vec[3].c_str()));
  else if(!syscall_type.compare("sync"))
    res = new VFSSync(atof(vec[0].c_str())*1000);
  else if(!syscall_type.compare("dropcache"))
    res = new VFSDropCache(atof(vec[0].c_str())*1000);
  else if(!syscall_type.compare("reset_stats"))
  {
      Simulation *s = Simulation::getInstance();
      s->resetStats();
      res = new VFSResetStats(atof(vec[0].c_str())*1000);
  }
  else if(!syscall_type.compare("close"))
    res = new VFSClose(atof(vec[0].c_str())*1000, atoi(vec[2].c_str()));
  else
    ERROR("Unknown event type on line " + nts(_current_line) + " of trace file(" + vec[1] + ")");

  assert(res != NULL);

  return res;
}
