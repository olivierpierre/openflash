#ifndef FTL_SYTEM_HPP
#define FTL_SYTEM_HPP

#include "FlashTranslationLayer.hpp"
#include "FlashBuffer.hpp"

class FtlSystem
{
public:
  static FtlSystem *getInstance();
  static void kill();

  PpcValF2 ftlSysRead(uint64_t address, uint32_t size);
  PpcValF2 ftlSysWrite(uint64_t address, uint32_t size);
  PpcValF2 ftlSysTrim(uint64_t address, uint32_t size);

private:
  FlashTranslationLayer *_ftl;
  FlashBuffer *_buffer;

  Address sectorAddrToPage (uint64_t sector);
  vector<Address> sectorRangeToPages (uint64_t start, uint32_t size);

  FtlSystem(FlashTranslationLayer *ftl) : _ftl(ftl), _buffer(NULL) {};
  FtlSystem(FlashTranslationLayer *ftl, FlashBuffer *buffer) : _ftl(ftl), _buffer(buffer) {};
  ~FtlSystem();
  static FtlSystem *_singleton;
	
};

#endif /* FTL_SYTEM_HPP */
