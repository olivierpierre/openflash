#ifndef FLASH_BUFFER_HPP
#define FLASH_BUFFER_HPP

#include "FlashTranslationLayer.hpp"

class FlashBuffer
{
public:
  static FlashBuffer * getInstance();
  static void kill();
  virtual ~FlashBuffer()=0;

  virtual PpcValF2 fbRead(Address address)=0;
  virtual PpcValF2 fbWrite(Address address)=0;
  virtual PpcValF2 fbTrim(Address address)=0;

protected:
	FlashBuffer(FlashTranslationLayer *ftl) : _ftl(ftl) {};
	FlashTranslationLayer *_ftl;

private:
	static FlashBuffer *_singleton;
};

#endif /* FLASH_BUFFER_HPP */
