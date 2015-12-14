/**
 * VFS representation
 */

#include <iostream>
#include "../FlashFileSystem.hpp"
#include "ReadAhead.hpp"
#include "PageCache.hpp"
#include "VfsStats.hpp"

using namespace std;

/**
 * TODO :
 * in order to support simulating several processes reading 1 file, we have to implement the notion of
 * file descriptor. Indeed, a readahead object is related to one file descriptor and (not one inode). For example
 * when 2 processes open the same file there are 2 file descriptors and then 2 read ahead objects. With our
 * current implementation (e.g. an opened file is only identified with its inode) we cannot simulate that.
 * So for now an openned file must first be closed before being re-openned again.
 */

class VirtualFileSystem
{
public:
  static VirtualFileSystem * getInstance();
  static void kill();

  ~VirtualFileSystem();
  PpcValF vfsRead(uint32_t inode, uint32_t offset, uint32_t count);
  PpcValF vfsWrite(uint32_t inode, uint32_t offset, uint32_t count);
  PpcValF vfsRemove(uint32_t inode);
  PpcValF vfsOpen(uint32_t inode, bool truncate, bool create, int name_len);
  PpcValF vfsCreate(uint32_t inode, int name_len);
  PpcValF vfsTruncate(uint32_t inode, uint32_t length);
  PpcValF vfsRename(uint32_t inode, int new_name_len);
  PpcValF vfsSync();
  PpcValF vfsClose(uint32_t inode);
  PpcValF vfsDropCache();
//  int indexInPageCache(uint32_t inode, uint32_t page_index);

  VfsStats *getStats() {return _stats;};

private:
  FlashFileSystem *_ffs;
//  vector<LinuxPage> _page_cache;
  PageCache *_page_cache;
  vector<uint32_t> _openned_inodes;
  vector<ReadAhead *> _ra_objects;
//  uint32_t _max_page_cache_size_pages;
  uint32_t _max_ino;

  double _base_readpage_cpu_energy_overhead;
  double _base_readpage_mem_energy_overhead;
  double _base_writepage_cpu_energy_overhead;
  double _base_writepage_mem_energy_overhead;
  double _base_readpage_overhead;

  VfsStats *_stats;

//  int insertInPageCache(uint32_t inode, uint32_t page_index);
//  int setLruAccessed(uint32_t inode, uint32_t page_index);
//  int evictPageFromPageCache();
  bool isOpen(uint32_t inode_num);
  ReadAhead *getRaPtr(uint32_t inode_num);

  VirtualFileSystem(FlashFileSystem *ffs, uint64_t page_cache_size_byte);
  static VirtualFileSystem *_singleton;
};
