#include "Hftl.hpp"

Hftl::Hftl () : FlashTranslationLayer(FlashSystem::getInstance())
{
  // init Hftl ...
}

Hftl::~Hftl ()
{
  // exit Hftl ...
}

PpcValF2 Hftl::ftlRead (Address address)
{
  PpcValF2 res = {0, 0};

  // manage read operation

  return res;
}

PpcValF2 Hftl::ftlWrite (Address address)
{
  PpcValF2 res = {0, 0};

  // manage write operation

  return res;
}

PpcValF2 Hftl::ftlTrim (Address address)
{
  PpcValF2 res = {0, 0};
  ERROR("Hftl trim not implemented yet");
  return res;
}
