/*
 * ReadAhead.cpp
 *
 *  Created on: 15 avr. 2014
 *      Author: armadeus
 */

#include "ReadAhead.hpp"
#include <cassert>
#include <cmath>
#include "../FlashFileSystem.hpp"

int ReadAhead::maxReadAheadSizeInPages = 32;

ReadAhead::ReadAhead (uint32_t inode_num)
{
  _inode_num = inode_num;

  _start = 0;
  _size = 0;
  _async_size = 0;
  _prev_pos = -1;

  _page_cache = PageCache::getInstance();
  _ffs = FlashFileSystem::getInstance();
}

/**
 * This is the core of the RA algorithm
 * kernel equivalent : ondemand_readahead
 */
PpcValF ReadAhead::onDemandRA (uint32_t offset, uint32_t req_size, bool async)
{
  PpcValF res = {0, 0};
  uint32_t max = getMaxReadAheadSize();

#ifdef VERBOSE
  cout << "[RA] Ondemand Read-Ahead" << endl;
#endif /* VERBOSE */

  /* start of file */
  if(!offset)
  {
#ifdef VERBOSE
  cout << "[RA] Start of file, reset window" << endl;
#endif /* VERBOSE */
    goto initial_readahead;
  }

  /* RA marker hit */
  if ((offset == (_start + _size - _async_size) || (offset == _start + _size)))
  {
#ifdef VERBOSE
  cout << "[RA] Marker hit, increase size" << endl;
#endif /* VERBOSE */
    _start += _size;
    _size = getNextRaSize(max);
    _async_size = _size;
    goto readit;
  }

  /* RA marker hit with invalid ra state */
  if (async)
  {
#ifdef VERBOSE
  cout << "[RA] Marker hit with invalid size" << endl;
#endif /* VERBOSE */
    uint32_t start_index = radixTreeNextHole(offset+1,max);

    if (!start_index || start_index - offset > max)
      return res;

#ifdef VERBOSE
  cout << "[RA] Increase size anyway" << endl;
#endif /* VERBOSE */

    _start = start_index;
    _size = start_index - offset;	/* old async_size */
    _size += req_size;
    _size = getNextRaSize(max);
    _async_size = _size;

    goto readit;
  }

  /* read too big */
  if (req_size > max)
  {
#ifdef VERBOSE
  cout << "[RA] Read too big, reset window" << endl;
#endif /* VERBOSE */
    goto initial_readahead;
  }

  /* sequential cache miss ? */
  /* ok there was a mistake in the kernel code here, which was fixed in linux 3.13
   * This alter the readahead behavior on random workloads
   * as we validate against linux 2.6.3xx, we provide a way to keep the incorrect behavior
   * through the parameter functional_model.vfs.read_ahead_sequential_cache_miss_bug
   */
  bool sequential_cache_miss;
  if(Param::getInstance()->getBool("functional_model.vfs.read_ahead_sequential_cache_miss_bug"))
    sequential_cache_miss = (offset - (_prev_pos / LINUX_PAGE_SIZE) <= 1);
  else
    sequential_cache_miss = (offset - ((uint32_t)_prev_pos / LINUX_PAGE_SIZE) <= 1UL);

  if(sequential_cache_miss)
  {
#ifdef VERBOSE
  cout << "[RA] Seq. cache miss, reset window" << endl;
#endif /* VERBOSE */
    goto initial_readahead;
  }

  /* look in the PC for traces of seq. streams */
  if (tryContextReadAhead(offset, req_size, max))
  {
#ifdef VERBOSE
  cout << "[RA] perform context based RA" << endl;
#endif /* VERBOSE */
    goto readit;
  }

  /* small rand access */
#ifdef VERBOSE
  cout << "[RA] Small & random access" << endl;
#endif /* VERBOSE */
  res = res + doRa(offset, req_size, 0);
  return res;

initial_readahead:
  _start = offset;
  _size = getInitRaSize(req_size, maxReadAheadSizeInPages);
  _async_size = _size > req_size ? _size - req_size : _size;

readit:
  if (offset == _start && _size == _async_size)
  {
    _async_size = getNextRaSize(max);
    _size += _async_size;
  }

  res = res + doRa(_start, _size, _async_size);

  return res;
}

/**
 * Ask ffs for the pages
 */
PpcValF ReadAhead::doRa (uint32_t ra_start, uint32_t ra_size, uint32_t ra_async_size)
{
  PpcValF res = {0, 0};

#ifdef VERBOSE
  cout << "[RA] ino#" << _inode_num << ", Reading @" << ra_start << " for " <<
      ra_size << " pages" << endl;
  if(ra_async_size)
    cout << "[RA] async_size : " << ra_async_size << endl;
#endif /* VERBOSE */

  for(uint32_t page_index = ra_start; page_index < (ra_start + ra_size); page_index++)
  {
    /* is the page in the page cache ? probably not */
    if(_page_cache->findInPageCache(_inode_num, page_index) == NULL)
    {
      int ret;
      res = res + _ffs->ffsReadPage(_inode_num, page_index, &ret);
      if(ret) // ignore I/O errors
      {
	_page_cache->insertInPageCache(_inode_num, page_index);
	// modif : we put the ra flag only on pages which are not already in the PC
	if(page_index == (ra_start + ra_size - ra_async_size))
	{
#ifdef VERBOSE
cout << "[RA] setting ra flag on #ino" << _inode_num << " page " << page_index << endl;
#endif /* VERBOSE */
	  LinuxPage *tmp = _page_cache->findInPageCache(_inode_num, page_index);
	  tmp->setReadAheadFlag();
	}
      }
    }



  }

  return res;
}

/**
 * Called by VFS on a page cache miss
 * offset is a linux page index in the file
 * req_size if the number of pages to read
 */
PpcValF ReadAhead::pageCacheSyncRA(uint32_t offset, uint32_t req_size)
{
#ifdef VERBOSE
  cout << "[RA] Sync RA ino#" << _inode_num << " @" << offset << " for " << req_size << " pages" << endl;
#endif /* VERBOSE */
  return onDemandRA(offset, req_size, false);
}

/**
 * Called by VFS on a page cache hit on a page with RA flag set
 */
PpcValF ReadAhead::pageCacheAsyncRA(uint32_t offset, uint32_t req_size)
{
#ifdef VERBOSE
  cout << "[RA] ASync RA ino#" << _inode_num << " @" << offset << " for " << req_size << " pages" << endl;
#endif /* VERBOSE */
  return onDemandRA(offset, req_size, true);
}

/**
 * Get initial size for RA (when RA is set for the first time or reset)
 * (nearly) copy pasted from linux source
 */
uint32_t ReadAhead::getInitRaSize (uint32_t req_size, uint32_t max)
{
  uint32_t newsize = pow(2, ceil(log(req_size)/log(2)));

  if (newsize <= max / 32)
    newsize = newsize * 4;
  else if (newsize <= max / 4)
    newsize = newsize * 2;
  else
    newsize = max;

  return newsize;
}

/**
 * Used when ra size is ramped up
 * same as above
 */
uint32_t ReadAhead::getNextRaSize(uint32_t max)
{
  uint32_t cur = _size;
  uint32_t newsize;

  if (cur < max / 16)
    newsize = 4 * cur;
  else
    newsize = 2 * cur;

  return min(newsize, max);
}

/**
 * Return the index of the next page which is not present in the page cache, the page belonging
 * to the range min,
 */
uint32_t ReadAhead::radixTreeNextHole (uint32_t offset, uint32_t range)
{
  assert(offset != 0);

  vector<LinuxPage *> file_pages = _page_cache->getOrderedPagesForFile(_inode_num);

  int min_index = -1;
  for(int i=0; i<(int)file_pages.size(); i++)
    if(file_pages[i]->getPageIndex() == offset)
      min_index = i;

  if(min_index == -1)
    return offset;

  int max_offset = min((uint32_t)(file_pages.size()-1), min_index + range);
  for(int i=min_index; i<max_offset; i++)
    if(file_pages[i+1]->getPageIndex() != file_pages[i]->getPageIndex()+1)
      break;
    else
      offset++;

  return offset+1;
}

/**
 * TODO
 */
int ReadAhead::tryContextReadAhead (uint32_t offset, uint32_t req_size,
				uint32_t max)
{
  uint32_t size = countBackwardConsecutive(offset-1, max);

  if(!size)
    return 0;

  if (size >= offset)
    size *= 2;

  _start = offset;
  _size = getInitRaSize(size + req_size, max);
  _async_size = _size;

  return 1;
}

/**
 * count backwards the number of cached consecutive pages from @offset-1 to @offset-max
 */
uint32_t ReadAhead::countBackwardConsecutive (uint32_t offset, uint32_t max)
{
  vector<LinuxPage *> pages = _page_cache->getOrderedPagesForFile(_inode_num);
  int index = -1;
  int res = 0;

  for(int i=0; i<(int)pages.size(); i++)
    if(pages[i]->getPageIndex() >= offset)
    {
      index = i;
      break;
    }

  if(index == -1 || pages[index]->getPageIndex() != offset)
    return 0;

  res = 1;
  for(int i=index; i>0; i--)
    if(pages[i-1]->getPageIndex() == pages[i]->getPageIndex() - 1)
      res++;
    else
      break;

  return res;
}
