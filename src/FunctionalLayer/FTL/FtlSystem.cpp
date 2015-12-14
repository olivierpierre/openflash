#include "FtlSystem.hpp"

FtlSystem * FtlSystem::_singleton = NULL;

FtlSystem* FtlSystem::getInstance ()
{
  if(_singleton == NULL)
  {
    if(Param::getInstance()->getString("functional_model.ftl.flash_buffer").compare("none"))
    {
      FlashBuffer *buf = FlashBuffer::getInstance();
      FlashTranslationLayer *ftl = FlashTranslationLayer::getInstance();
      _singleton = new FtlSystem(ftl, buf);
    }
    else
    {
      FlashTranslationLayer *ftl = FlashTranslationLayer::getInstance();
      _singleton = new FtlSystem(ftl);
    }
  }

  return _singleton;
}

FtlSystem::~FtlSystem ()
{
  _ftl->kill();
  if(_buffer != NULL)
    _buffer->kill();
}

void FtlSystem::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

PpcValF2 FtlSystem::ftlSysRead (uint64_t address, uint32_t size)
{
  PpcValF2 res = {0, 0};
  vector<Address> addresses = sectorRangeToPages(address, size);

  for(int i=0; i<(int)addresses.size(); i++)
    if(_buffer != NULL)
      res = res + _buffer->fbRead(addresses[i]);
    else
      res = res + _ftl->ftlRead(addresses[i]);

  return res;
}

PpcValF2 FtlSystem::ftlSysWrite (uint64_t address, uint32_t size)
{
  PpcValF2 res = {0, 0};
  vector<Address> addresses = sectorRangeToPages(address, size);

  for(int i=0; i<(int)addresses.size(); i++)
    if(_buffer != NULL)
      res = res + _buffer->fbWrite(addresses[i]);
    else
      res = res + _ftl->ftlWrite(addresses[i]);

  return res;
}

PpcValF2 FtlSystem::ftlSysTrim (uint64_t address, uint32_t size)
{
  PpcValF2 res = {0, 0};
  vector<Address> addresses = sectorRangeToPages(address, size);

  for(int i=0; i<(int)addresses.size(); i++)
    if(_buffer != NULL)
      res = res + _buffer->fbTrim(addresses[i]);
    else
      res = res + _ftl->ftlTrim(addresses[i]);

  return res;
}

/**
 * Convert an address in sectors to Address object
 */
Address FtlSystem::sectorAddrToPage (uint64_t sector)
{
  int page = (sector / PAGE_SIZE_SECTORS) % PAGES_PER_BLOCK;
  int block = (sector / BLOCK_SIZE_SECTORS) % BLOCKS_PER_PLANE;
  int plane = (sector / PLANE_SIZE_SECTORS) % PLANES_PER_LUN;
  int lun = (sector / LUN_SIZE_SECTORS) % LUNS_PER_CHANNEL;
  int channel = (sector / CHANNEL_SIZE_SECTORS);

  return Address(channel, lun, plane, block, page);

}

/**
 * Convert a sector range in x addresses, each one being one of the
 * flash pages covered by the range
 */
vector<Address> FtlSystem::sectorRangeToPages (uint64_t start, uint32_t size)
{
  vector<Address> res;

  uint64_t aligned_start = (start/PAGE_SIZE_SECTORS)*PAGE_SIZE_SECTORS;
  uint64_t aligned_end = ((start + size - 1)/PAGE_SIZE_SECTORS)*PAGE_SIZE_SECTORS;

  for(uint64_t i = aligned_start; i <= aligned_end; i+=PAGE_SIZE_SECTORS)
    res.push_back(sectorAddrToPage(i));

  return res;
}
