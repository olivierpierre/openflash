#ifndef TRACE_PARSER_HPP
#define TRACE_PARSER_HPP

#include <string>

#include "../Event.hpp"

using namespace std;

/**
 * Infos about a FFS trace, used for the warmup
 */
typedef struct
{
  vector<uint32_t> read_inode_list;
  vector<uint32_t> read_inode_sizes;
} ffs_trace_info_t;

/**
 * Infos on a FTL trace, used for the warmup
 */
typedef struct
{
  //TODO
} ftl_trace_info_t;

class TraceParser
{
public:
  static TraceParser *getInstance();
  static void kill();

  virtual ~TraceParser();
  virtual Event *getNextEvent()=0;
  virtual uint32_t getCurrentLine();
  virtual void print_progress()=0;

protected:
  TraceParser(string file);
  ifstream *_file;
  string _file_path;
  bool _print_progress;

private:
  static TraceParser *_singleton;
};

#endif /* TRACE_PARSER_HPP */
