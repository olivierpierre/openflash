#ifndef DFTL_HPP
#define DFTL_HPP

#include "../FlashTranslationLayer.hpp"

class Dftl : public FlashTranslationLayer
{
public:

  Dftl ();
  virtual ~Dftl ();

  PpcValF2 ftlRead(Address address);       // Basic operations : read
  PpcValF2 ftlWrite(Address address);      // write
  PpcValF2 ftlTrim(Address address);       // trim

private:
};

#endif /* DFTL_HPP */
