#ifndef BPLRU_HPP
#define BPLRU_HPP

#include "../FlashBuffer.hpp"

class Bplru : public FlashBuffer
{
public:
	Bplru(FlashTranslationLayer *ftl) : FlashBuffer(ftl) {};
	~Bplru() {};

  PpcValF2 fbRead(Address address);
  PpcValF2 fbWrite(Address address);
  PpcValF2 fbTrim(Address address);

private:


};

#endif /* BPLRU_HPP */
