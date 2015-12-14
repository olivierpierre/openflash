#ifndef PAGE_CACHE_HPP
#define PAGE_CACHE_HPP

#include <stdint.h>
#include <vector>

#include "../../FunctionalGlobals.hpp"

using namespace std;

class LinuxPage
{
public:
  LinuxPage(uint32_t inode_num, uint32_t page_index);
  uint32_t getInodeNum() const {return _inode_num;};
  uint32_t getPageIndex() const {return _page_index;};
  bool getReadAheadFlag() const {return _readahead_flag;};
  void setReadAheadFlag() {_readahead_flag = true;};
  void removeReadAheadFlag() {_readahead_flag = false;};
private:
  uint32_t _inode_num;
  uint32_t _page_index;
  bool _readahead_flag;
};

class PageCache
{
public:
  static PageCache *getInstance();
  static void kill();
  LinuxPage *findInPageCache(uint32_t inode, uint32_t page_index);
  int insertInPageCache(uint32_t inode, uint32_t page_index);
  int setLruAccessed(uint32_t inode, uint32_t page_index);
  PpcValF drop();
  vector<LinuxPage *> getOrderedPagesForFile(uint32_t inode);


private:
  vector<LinuxPage *> _page_cache;
  uint32_t _max_size_pages;
  int evictPageFromPageCache();
  int getIndexInPageCache(uint32_t inode, uint32_t page_index);
  int internalRemovePC(int index_in_pc_vector);

  PageCache(uint64_t size_bytes);
  virtual ~PageCache();
  static PageCache *_singleton;

  // debug
  int sanityCheck(uint32_t inode, uint32_t inserted_index);
};

#endif /* PAGE_CACHE_HPP */
