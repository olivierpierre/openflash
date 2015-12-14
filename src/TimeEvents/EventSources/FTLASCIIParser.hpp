#ifndef FTL_ASCII_PARSER_HPP
#define FTL_ASCII_PARSER_HPP

#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>

#include "TraceParser.hpp"
#include "../../FlashLayer/Address.hpp"

using namespace std;

class FTLASCIIParser : public TraceParser
{
public:
  FTLASCIIParser(string file);
  Event *getNextEvent();
  void print_progress();

private:
  bool mustNotParse(string line);
  Event *allocateEventFromAsciiLine(string& line);
};

#endif /* FTL_ASCII_PARSER_HPP */
