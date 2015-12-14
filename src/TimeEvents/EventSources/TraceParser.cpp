#include <fstream>
#include <iostream>

#include "../../Globs/Common.hpp"
#include "TraceParser.hpp"
#include "FFSCSVParser.hpp"
#include "FTLASCIIParser.hpp"

TraceParser *TraceParser::_singleton = NULL;

TraceParser* TraceParser::getInstance ()
{
  if(_singleton == NULL)
  {
    if(Param::getInstance()->getString("simulator.trace.mode").compare("file"))
      ERROR("Parameter simulator.trace.mode should be 'file' when requesting a traceparser");

    if(!Param::getInstance()->getString("simulator.trace.parser").compare("ffs_csv"))
      _singleton = new FFSCSVParser(Param::getInstance()->getString("simulator.trace.path"));
    else if(!Param::getInstance()->getString("simulator.trace.parser").compare("ftl_ascii"))
      _singleton = new FTLASCIIParser(Param::getInstance()->getString("simulator.trace.path"));
    else
      ERROR("Unknown parser " + Param::getInstance()->getString("simulator.trace.parser"));
  }

  return _singleton;
}

void TraceParser::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

uint32_t TraceParser::getCurrentLine ()
{
  WARNING("GetCurrentLine called on parsed not supporting this function ...");
  return 0xFFFFFFFF;
}

TraceParser::TraceParser (string file)
{
  _file_path = file;
  _file = new ifstream(file.c_str(), ios_base::in);
  if(!_file->good())
    ERROR("Cannot open tracefile " + file);

  _print_progress = Param::getInstance()->getBool("simulator.print_progress");

#ifdef VERBOSE
  cout << "[Parser] Openned " << _file_path << endl;
#endif /* VERBOSE */

}

TraceParser::~TraceParser()
{
#ifdef VERBOSE
  cout << "[Parser] Closing " << _file_path << endl;
#endif /* VERBOSE */

  _file->close();
  delete _file;
  _file = NULL;
}
