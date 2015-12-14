#include "Fast.hpp"

Fast::Fast () : FlashTranslationLayer(FlashSystem::getInstance())
{
  // init Fast ...
}

Fast::~Fast ()
{
  // exit Fast ...
}

PpcValF2 Fast::ftlRead (Address address)
{
  PpcValF2 res = {0, 0};

  // manage read operation

  return res;
}

PpcValF2 Fast::ftlWrite (Address address)
{
  PpcValF2 res = {0, 0};

  // manage write operation

  return res;
}

PpcValF2 Fast::ftlTrim (Address address)
{
  PpcValF2 res = {0, 0};
  ERROR("Fast trim not implemented yet");
  return res;
}
