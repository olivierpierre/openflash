/*
 * NandDriver.cpp
 *
 *  Created on: 19 mars 2014
 *      Author: pierre
 * 
 * Functional model implementation for the Linux MTD generic NAND driver
 */

#include "NandDriver.hpp"

#include <cstdlib>
#include "../../FlashLayer/FlashCmdList.hpp"


NandDriver *NandDriver::_singleton = NULL;

NandDriver::NandDriver (FlashSystem *f) : _read_cache(Address(0,0,0,0,0))
{
  _read_time_overhead = Param::getInstance()->getDouble("ppc_models.nand_driver.read_time_overhead");
  _write_time_overhead = Param::getInstance()->getDouble("ppc_models.nand_driver.write_time_overhead");
  _erase_time_overhead = Param::getInstance()->getDouble("ppc_models.nand_driver.erase_time_overhead");

  _read_mem_power = Param::getInstance()->getDouble("ppc_models.nand_driver.read_mem_power");
  _write_mem_power = Param::getInstance()->getDouble("ppc_models.nand_driver.write_mem_power");
  _erase_mem_power = Param::getInstance()->getDouble("ppc_models.nand_driver.erase_mem_power");
  _read_cpu_power = Param::getInstance()->getDouble("ppc_models.nand_driver.read_cpu_power");
  _write_cpu_power = Param::getInstance()->getDouble("ppc_models.nand_driver.write_cpu_power");
  _erase_cpu_power = Param::getInstance()->getDouble("ppc_models.nand_driver.erase_cpu_power");

  _stats = new MtdStats(Param::getInstance()->isParamPresent("outputs.mtd"));

  invalidateReadCache();

  _f = f;
}

NandDriver::~NandDriver ()
{
  delete _stats;
}

/**
 * Singleton stuff
 */
NandDriver *NandDriver::getInstance()
{
  if(_singleton == NULL)
    {
      _singleton = new NandDriver(FlashSystem::getInstance());
    }

  return _singleton;
}

/**
 * Singleton stuff
 */
void NandDriver::kill()
{
  if(_singleton != NULL)
    delete _singleton;
}

/**
 * Interface : read a flash page
 */
PpcValF NandDriver::readPage (Address addr)
{
  PpcValF res = {0, 0};

  /* read cache hit ? */
  if(addr == _read_cache && _read_cache_valid)
  {
#ifdef VERBOSE
    cout << "[MTD] Read cache hit @" << addr << ", time taken: " << res.time << endl;
#endif /* VERBOSE */

    /* TODO lets say for now this consumes nothing */
    _stats->addMtdCallInfo(MTD_READ_HIT, addrToPageIndex(addr), res.time, 0, 0);

    return res;
  }

  LegacyRead lr(addr);

  res.time = lr.getTimeTaken();
  res.time += _read_time_overhead;
  /* on FFS mode we do not use the theoritical PC model (no lr.getEnergyConsumed) */
  res.e_mem = (res.time/1000000.0)*_read_mem_power;
  res.e_cpu = (res.time/1000000.0)*_read_cpu_power;

  // virer ca
//  cout << "driver read page exec time:" << res.time << endl;

  /* set this page as the last page read */
  setReadCache(addr);

#ifdef VERBOSE
  cout << "[MTD] page read @" << addr << " : " << res.time << " us" << endl;
#endif /* VERBOSE */

  _stats->addMtdCallInfo(MTD_READ, addrToPageIndex(addr), res.time, res.e_mem, res.e_cpu);

  return res;
}

/**
 * Interface : write a flash page
 */
PpcValF NandDriver::writePage (Address addr)
{
  PpcValF res;
  LegacyWrite lw(addr);

  res.time = lw.getTimeTaken();
  res.time += _write_time_overhead;
  /* on FFS mode we do not use the theoritical PC model (no lw.getEnergyConsumed) */
  res.e_mem = (res.time/1000000.0)*_write_mem_power;
  res.e_cpu = (res.time/1000000.0)*_write_cpu_power;


  /* invalidate read cache if needed */
  if(_read_cache == addr)
    invalidateReadCache();

#ifdef VERBOSE
  cout << "[MTD] page write @" << addr << " : " << res.time << " us" << endl;
#endif /* VERBOSE */

  _stats->addMtdCallInfo(MTD_WRITE, addrToPageIndex(addr), res.time, res.e_mem, res.e_cpu);

  return res;
}

/**
 * Interface : erase a flash block
 */
PpcValF NandDriver::eraseBlock (Address addr)
{
  PpcValF res;
  LegacyErase le(addr);

  res.time = le.getTimeTaken();
  res.time += _erase_time_overhead;
  /* on FFS mode we do not use the theoritical PC model (no lw.getEnergyConsumed) */
  res.e_mem = (res.time/1000000.0)*_erase_mem_power;
  res.e_cpu = (res.time/1000000.0)*_erase_cpu_power;

  /* invalidate read cache if needed TODO rewrite its ugly */
  if(addr.getBlock() == _read_cache.getBlock() &&
      addr.getPlane() == _read_cache.getPlane() &&
      addr.getLun() == _read_cache.getLun() &&
      addr.getChannel() == _read_cache.getChannel())
	invalidateReadCache();

#ifdef VERBOSE
  cout << "[MTD] block erase @" << addr << " : " << res.time << " us" << endl;
#endif /* VERBOSE */

  _stats->addMtdCallInfo(MTD_ERASE, addrToPageIndex(addr)/PAGES_PER_BLOCK, res.time, res.e_mem, res.e_cpu);

  return res;
}

/**
 * Convert an address in bytes to Address object
 */
Address NandDriver::byteAddrToPage (uint64_t byte_addr)
{
  int page = (byte_addr / PAGE_SIZE_BYTE) % PAGES_PER_BLOCK;
  int block = (byte_addr / BLOCK_SIZE_BYTE) % BLOCKS_PER_PLANE;
  int plane = (byte_addr / PLANE_SIZE_BYTE) % PLANES_PER_LUN;
  int lun = (byte_addr / LUN_SIZE_BYTE) % LUNS_PER_CHANNEL;
  int channel = (byte_addr / CHANNEL_SIZE_BYTE);

  return Address(channel, lun, plane, block, page);

}

/**
 * Convert a byte range in x addresses, each one being one of the
 * flash pages covered by the range
 */
vector<Address> NandDriver::byteRangeToPages (uint64_t start, uint32_t size)
{
  vector<Address> res;

  uint64_t aligned_start = (start/PAGE_SIZE_BYTE)*PAGE_SIZE_BYTE;
  uint64_t aligned_end = ((start + size - 1)/PAGE_SIZE_BYTE)*PAGE_SIZE_BYTE;

  for(uint64_t i = aligned_start; i <= aligned_end; i+=PAGE_SIZE_BYTE)
    res.push_back(byteAddrToPage(i));

  return res;
}

/**
 * Convert an address to an absolute page index
 */
uint64_t NandDriver::addrToPageIndex (Address addr)
{
  uint64_t page = addr.getPage();

  page += addr.getBlock()*PAGES_PER_BLOCK;
  page += addr.getPlane()*PAGES_PER_BLOCK*BLOCKS_PER_PLANE;
  page += addr.getLun()*PAGES_PER_BLOCK*BLOCKS_PER_PLANE*PLANES_PER_LUN;
  page += addr.getChannel()*PAGES_PER_BLOCK*BLOCKS_PER_PLANE*PLANES_PER_LUN*LUNS_PER_CHANNEL;

  return page;
}
