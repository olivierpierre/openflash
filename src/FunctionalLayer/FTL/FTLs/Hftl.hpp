#ifndef HFTL_HPP
#define HFTL_HPP

#include "../FlashTranslationLayer.hpp"

class Hftl : public FlashTranslationLayer
{
public:

  Hftl ();
  virtual ~Hftl ();

  PpcValF2 ftlRead(Address address);       // Basic operations : read
  PpcValF2 ftlWrite(Address address);      // write
  PpcValF2 ftlTrim(Address address);       // trim

private:
};

#endif /* HFTL_HPP */
