#include "Dftl.hpp"

Dftl::Dftl () : FlashTranslationLayer(FlashSystem::getInstance())
{
  // init DFTL ...
}

Dftl::~Dftl ()
{
  // exit DFTL ...
}

PpcValF2 Dftl::ftlRead (Address address)
{
  PpcValF2 res = {0, 0};

  // manage read operation

  return res;
}

PpcValF2 Dftl::ftlWrite (Address address)
{
  PpcValF2 res = {0, 0};

  // manage write operation

  return res;
}

PpcValF2 Dftl::ftlTrim (Address address)
{
  PpcValF2 res = {0, 0};
  ERROR("DFTL trim not implemented yet");
  return res;
}
