#include "Last.hpp"

Last::Last () : FlashTranslationLayer(FlashSystem::getInstance())
{
  // init Last ...
}

Last::~Last ()
{
  // exit Last ...
}

PpcValF2 Last::ftlRead (Address address)
{
  PpcValF2 res = {0, 0};

  // manage read operation

  return res;
}

PpcValF2 Last::ftlWrite (Address address)
{
  PpcValF2 res = {0, 0};

  // manage write operation

  return res;
}

PpcValF2 Last::ftlTrim (Address address)
{
  PpcValF2 res = {0, 0};
  ERROR("Last trim not implemented yet");
  return res;
}
