#ifndef FLASH_TRANSLATION_LAYER_HPP
#define FLASH_TRANSLATION_LAYER_HPP

#include "../../FlashLayer/FlashLayer.hpp"
#include "../FunctionalGlobals.hpp"

class FlashTranslationLayer
{
public:
  static FlashTranslationLayer * getInstance();
  static void kill();
  virtual ~FlashTranslationLayer()=0;

  virtual PpcValF2 ftlRead(Address address)=0;
  virtual PpcValF2 ftlWrite(Address address)=0;
  virtual PpcValF2 ftlTrim(Address address)=0;

protected:
  FlashTranslationLayer(FlashSystem *f) : _f(f) {};
  FlashSystem *_f;

  PpcValF2 flashLegacyRead(Address address);
  PpcValF2 flashLegacyWrite(Address address);
  PpcValF2 flashLegacyErase(Address address);

private:
  static FlashTranslationLayer *_singleton;
};

#endif /* FLASH_TRANSLATION_LAYER_HPP */
