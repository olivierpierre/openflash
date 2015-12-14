#ifndef FFS_CSV_PARSER_HPP
#define FFS_CSV_PARSER_HPP

#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>

#include "TraceParser.hpp"

using namespace std;

class FFSCSVParser : public TraceParser
{
public:
  FFSCSVParser(string file);
  ~FFSCSVParser() {};
  Event *getNextEvent();
  void print_progress();

private:
  bool mustNotParse(string line);
  vector<string> breakCsvLine(string line, char sep=';');
  Event *allocateEventFromStringVector(vector<string> &vec);
  uint32_t _current_line;
  uint32_t _total_lines;
};

#endif /* FFS_CSV_PARSER_HPP */
