/*
 * ReadAhead.hpp
 *
 *  Created on: 15 avr. 2014
 *      Author: pierre
 */

#ifndef READAHEAD_HPP
#define READAHEAD_HPP

#include <vector>
#include <stdint.h>

#include "../../FunctionalGlobals.hpp"
#include "../FlashFileSystem.hpp"
#include "PageCache.hpp"

using namespace std;

class ReadAhead
{
public:
  ReadAhead(uint32_t inode_num);

  void setMaxReadAheadSize(int val) {maxReadAheadSizeInPages = val;};
  void setPrevPos(uint32_t val) {_prev_pos = val;};
  int getMaxReadAheadSize() {return maxReadAheadSizeInPages;};

  uint32_t getInode() {return _inode_num;};

  PpcValF pageCacheSyncRA(uint32_t offset, uint32_t req_size);
  PpcValF pageCacheAsyncRA(uint32_t offset, uint32_t req_size);

private:
    uint32_t _inode_num;
    uint32_t _start;
    uint32_t _size;
    uint32_t _async_size;
    long int _prev_pos;
    PageCache * _page_cache;
    FlashFileSystem *_ffs;

    PpcValF onDemandRA(uint32_t offset, uint32_t req_size, bool async);
    PpcValF doRa(uint32_t ra_start, uint32_t ra_size, uint32_t ra_async_size);
    uint32_t getInitRaSize(uint32_t req_size, uint32_t max);
    uint32_t getNextRaSize(uint32_t max);
    uint32_t radixTreeNextHole(uint32_t min, uint32_t max);
    int tryContextReadAhead(uint32_t offset, uint32_t req_size, uint32_t max);
    uint32_t countBackwardConsecutive(uint32_t offset, uint32_t max);

    static int maxReadAheadSizeInPages;

};

#endif /* READAHEAD_HPP */
