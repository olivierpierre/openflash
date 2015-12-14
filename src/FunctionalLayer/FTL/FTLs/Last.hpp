#ifndef LAST_HPP
#define LAST_HPP

#include "../FlashTranslationLayer.hpp"

class Last : public FlashTranslationLayer
{
public:

  Last ();
  virtual ~Last ();

  PpcValF2 ftlRead(Address address);       // Basic operations : read
  PpcValF2 ftlWrite(Address address);      // write
  PpcValF2 ftlTrim(Address address);       // trim

private:
};

#endif /* LAST_HPP */
