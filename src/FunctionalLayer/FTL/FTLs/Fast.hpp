#ifndef FAST_HPP
#define FAST_HPP

#include "../FlashTranslationLayer.hpp"

class Fast : public FlashTranslationLayer
{
public:

  Fast ();
  virtual ~Fast ();

  PpcValF2 ftlRead(Address address);       // Basic operations : read
  PpcValF2 ftlWrite(Address address);      // write
  PpcValF2 ftlTrim(Address address);       // trim

private:
};

#endif /* FAST_HPP */
