/*
 * NandDriver.hpp
 *
 *  Created on: 19 mars 2014
 *      Author: pierre
 */

#ifndef NANDDRIVER_HPP
#define NANDDRIVER_HPP

#include <stdint.h>

#include "../../FlashLayer/FlashSystem.hpp"
#include "../FunctionalGlobals.hpp"
#include "../../FlashLayer/Address.hpp"
#include "MtdStats.hpp"

/**
 * For now the sole purpose of the nand driver is to add an overhead on flash operations
 * and model the MTD read buffer
 */

class NandDriver
{
public:
  static NandDriver *getInstance();
  static void kill();
  virtual ~NandDriver();

  PpcValF readPage(Address addr, mtd_call_src_t src);
  PpcValF writePage(Address addr, mtd_call_src_t src);
  PpcValF eraseBlock(Address addr, mtd_call_src_t src);

  Address byteAddrToPage(uint64_t byte_addr);
  vector<Address> byteRangeToPages(uint64_t start, uint32_t size);

  MtdStats *getStats() {return _stats;};

private:
    Address _read_cache;			/* last page read */
    bool    _read_cache_valid;		/* is the read cache valid ? */

    double _read_time_overhead;
    double _write_time_overhead;
    double _erase_time_overhead;
    double _read_mem_power;
    double _write_mem_power;
    double _erase_mem_power;
    double _read_cpu_power;
    double _write_cpu_power;
    double _erase_cpu_power;

    MtdStats *_stats;

    FlashSystem *_f;

    NandDriver(FlashSystem *f);
    static NandDriver *_singleton;

    void setReadCache(Address addr) { _read_cache = addr; _read_cache_valid = true;};
    void invalidateReadCache() {_read_cache_valid = false;};

    uint64_t addrToPageIndex(Address addr);
};

#endif /* NANDDRIVER_HPP */
