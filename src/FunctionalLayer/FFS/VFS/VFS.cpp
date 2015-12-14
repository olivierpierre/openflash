#include "VFS.hpp"

#include "../../../TimeEvents/TimeEvents.hpp"

VirtualFileSystem *VirtualFileSystem::_singleton = NULL;

VirtualFileSystem* VirtualFileSystem::getInstance ()
{
  if(_singleton == NULL)
  {
    uint64_t page_cache_size_bytes = Param::getInstance()->getInt("functional_model.vfs.page_cache_size_bytes");
    FlashFileSystem *ffs = FlashFileSystem::getInstance();

    _singleton = new VirtualFileSystem(ffs, page_cache_size_bytes);
  }

  return _singleton;
}

void VirtualFileSystem::kill ()
{
  if(_singleton == NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

bool VirtualFileSystem::isOpen (uint32_t inode_num)
{
  for(int i=0; i<(int)_openned_inodes.size(); i++)
    if(_openned_inodes[i] == inode_num)
      return true;
  return false;
}

/**
 * Return ra object associated to an open file
 * NULL if not found
 */
ReadAhead* VirtualFileSystem::getRaPtr (uint32_t inode_num)
{
  for(int i=0; i<(int)_ra_objects.size(); i++)
    if(_ra_objects[i]->getInode() == inode_num)
      return _ra_objects[i];
  return NULL;
}

VirtualFileSystem::VirtualFileSystem(FlashFileSystem* ffs, uint64_t page_cache_size_byte) : _ffs(ffs)
{
  _max_ino = 1;

  _page_cache = PageCache::getInstance();

  _base_readpage_cpu_energy_overhead = Param::getInstance()->getDouble("functional_model.vfs.base_readpage_cpu_energy_overhead");
  _base_readpage_mem_energy_overhead = Param::getInstance()->getDouble("functional_model.vfs.base_readpage_mem_energy_overhead");
  _base_writepage_cpu_energy_overhead = Param::getInstance()->getDouble("functional_model.vfs.base_writepage_cpu_energy_overhead");
  _base_writepage_mem_energy_overhead = Param::getInstance()->getDouble("functional_model.vfs.base_writepage_mem_energy_overhead");
  _base_readpage_overhead = Param::getInstance()->getDouble("functional_model.vfs.base_readpage_overhead");

  /** create the pdflush deamon */
  PdFlush *pdf = new PdFlush(Param::getInstance()->getDouble("functional_model.vfs.pdflush_frequency"));
  EventProcessor::getInstance()->insertAsyncEvent(*pdf);
  delete pdf;

  _stats = new VfsStats(Param::getInstance()->isParamPresent("outputs.vfs"));
}

VirtualFileSystem::~VirtualFileSystem()
{
  delete _stats;
}

/**
 * Read
 * kernel equivalent : do_generic_file_read
 */
PpcValF VirtualFileSystem::vfsRead (uint32_t inode, uint32_t offset, uint32_t count)
{
  PpcValF res = {0, 0};
  bool ra_on = Param::getInstance()->getBool("functional_model.vfs.read_ahead");

#ifdef VERBOSE
  cout << "[VFS] VfsRead ino: " << inode << ", offset:" << offset << ", count:" << count << endl;
#endif /* VERBOSE */

  if(!isOpen(inode))
    ERROR("Calling vfs read on non open file (ino#" + nts(inode) + ")");

  int first_page_index = offset / LINUX_PAGE_SIZE;
  int last_page_index = (offset+count-1) / LINUX_PAGE_SIZE;

  for(int i=first_page_index; i<=last_page_index; i++)
  {
    LinuxPage *page = _page_cache->findInPageCache(inode, i);
    if(page != NULL)
    {
      // mark lru accessed for that page
      _page_cache->setLruAccessed(inode, i);

      /* readahead ? */
      if(ra_on && page->getReadAheadFlag())
      {
	page->removeReadAheadFlag();
	res = res + getRaPtr(inode)->pageCacheAsyncRA(i, last_page_index - i +1);
      }

#ifdef VERBOSE
cout << "[VFS] PC Hit (" << inode << ", " << i << ")" << endl;
#endif /* VERBOSE */

      // read from the page cache ...
    }
    else
    {
      // read from storage media

      /* readahead ? */
      if(ra_on)
	res = res + getRaPtr(inode)->pageCacheSyncRA(i, last_page_index - i +1);

      /* no RA or RA could not put the page in PC */
      if((page = _page_cache->findInPageCache(inode, i)) == NULL)
      {
	int ret;
	res = res + _ffs->ffsReadPage(inode, i, &ret);

	if(!ret)
	  ERROR("Read error at filesystem level, probably read @ > file size");

	_page_cache->insertInPageCache(inode, i);
      }
    }

//    /* Add power consumption */
//    res.e_cpu += _base_readpage_cpu_energy_overhead/1000000.0;
//    res.e_mem += _base_readpage_mem_energy_overhead/1000000.0;
//
//    /* per page timing overhead */
//    res.time += _base_readpage_overhead;
  }

      res.e_cpu += (_base_readpage_cpu_energy_overhead/1000000.0)*(count/LINUX_PAGE_SIZE);
      res.e_mem += (_base_readpage_mem_energy_overhead/1000000.0)*(count/LINUX_PAGE_SIZE);

      /* per page timing overhead */
      res.time += (_base_readpage_overhead)*double(((double)(count)/(double)(LINUX_PAGE_SIZE)));

  if(ra_on)
    getRaPtr(inode)->setPrevPos(offset);

  _stats->addVfsCallInfo(VFS_READ, res.time, res.e_cpu, res.e_mem);

//  cout << "VFSRead T:" << res.time/1000000 << " s, e_mem: " << res.e_mem << ", e_cpu: " <<
//      res.e_cpu << endl;

  return res;
}

/**
 * TODO writeback
 * For now synchronous writes only with JFFS2
 */
PpcValF VirtualFileSystem::vfsWrite (uint32_t inode, uint32_t offset, uint32_t count)
{
  PpcValF res = {0, 0};

#ifdef VERBOSE
  cout << "[VFS:" << TIME_NOW << "] received vfsWrite ino:" << inode << ", offset:" << offset << ", count: " << count << endl;
#endif /* VERBOSE */

  if(!isOpen(inode))
      ERROR("Calling vfs write on non open file (ino#" + nts(inode) + ")");

  int first_page_index = offset / LINUX_PAGE_SIZE;
  int last_page_index = (offset + count -1) / LINUX_PAGE_SIZE;

  int written = 0;
  for(int i=first_page_index; i<=last_page_index; i++)
  {
    uint32_t offset_in_page = offset % LINUX_PAGE_SIZE;
    uint32_t count_in_page = min(count-written, LINUX_PAGE_SIZE - offset_in_page);

#ifdef VERBOSE
    cout << "[VFS] sending ffsWrite ino:" << inode << ", pg index:" << i << ", offset:" << offset_in_page <<
	", count:" << count_in_page << endl;
#endif /* VERBOSE */

    res = res + _ffs->ffsWriteBegin(inode, i, offset_in_page, count_in_page);
    res = res + _ffs->ffsWriteEnd(inode, i, offset_in_page, count_in_page);

    /* Put the page in the PC after the write operation */
    if(_page_cache->findInPageCache(inode, i) == NULL)
      _page_cache->insertInPageCache(inode, i);		/* will be marked as LRU by being inserted */
    else
      _page_cache->setLruAccessed(inode, i);

    offset += count_in_page;
    written += count_in_page;

    // PC overhead
    res.e_mem += _base_writepage_mem_energy_overhead;
    res.e_cpu += _base_writepage_cpu_energy_overhead;
  }

  _stats->addVfsCallInfo(VFS_WRITE, res.time, res.e_cpu, res.e_mem);

  return res;
}

PpcValF VirtualFileSystem::vfsRemove (uint32_t inode)
{
  PpcValF res = {0, 0};

  if(isOpen(inode))
      ERROR("Calling vfs remove on open file (ino#" + nts(inode) + ")");

#ifdef VERBOSE
  cout << "[VFS] VfsRemove ino:" << inode << " received" << endl;
#endif /* VERBOSE */

  res = res + _ffs->ffsRemove(inode);

  return res;
}

PpcValF VirtualFileSystem::vfsOpen (uint32_t inode, bool truncate, bool create,
			    int name_len)
{
  PpcValF res = {0, 0};

  if(isOpen(inode))
      ERROR("Calling vfs open on open file (ino#" + nts(inode) + ")");

  /* if ra on, create ra object */
  if(Param::getInstance()->getBool("functional_model.vfs.read_ahead"))
  {
    ReadAhead *ra = new ReadAhead(inode);
    _ra_objects.push_back(ra);
  }

  /* add to opened file list */
  _openned_inodes.push_back(inode);

  res = res + _ffs->ffsOpen(inode, truncate, create, name_len);

  return res;
}

PpcValF VirtualFileSystem::vfsCreate (uint32_t inode, int name_len)
{
  PpcValF res = {0, 0};

#ifdef VERBOSE
  cout << "[VFS] Creating ino:" << inode << ", namelen:" << name_len << endl;
#endif /* VERBOSE */

  if(isOpen(inode))
      ERROR("Calling vfs create on existing and open file (ino#" + nts(inode) + ")");

  if(! (inode > _max_ino))
    ERROR("Creating file with ino#(" + nts(inode) +
              ") <= current max ino number (" + nts(_max_ino) + ")");

  _max_ino = inode;

  res = res + _ffs->ffsCreate(inode, name_len);

  return res;
}

PpcValF VirtualFileSystem::vfsTruncate (uint32_t inode, uint32_t length)
{
  PpcValF res = {0, 0};

  if(isOpen(inode))
      ERROR("Calling vfs truncate on open file (ino#" + nts(inode) + ")");

  res = res + _ffs->ffsTruncate(inode, length);

  return res;
}

PpcValF VirtualFileSystem::vfsRename (uint32_t inode, int new_name_len)
{
  PpcValF res = {0, 0};

  res = res + _ffs->ffsRename(inode, new_name_len);

  return res;
}

PpcValF VirtualFileSystem::vfsSync ()
{
  PpcValF res = {0, 0};

  // TODO writeback ?

  res = res + _ffs->ffsSync();

  return res;
}

PpcValF VirtualFileSystem::vfsClose(uint32_t inode)
{
  PpcValF res = {0, 0};

  if(!isOpen(inode))
      ERROR("Calling vfs close on non open file (ino#" + nts(inode) + ")");

  /* remove file from opened file list */
  for(int i=0; i<(int)_openned_inodes.size(); i++)
    if(_openned_inodes[i] == inode)
      _openned_inodes.erase(_openned_inodes.begin() + i);

  /* if ra on, remove related object */
  if(Param::getInstance()->getBool("functional_model.vfs.read_ahead"))
    for(int i=0; i<(int)_ra_objects.size(); i++)
      if(_ra_objects[i]->getInode() == inode)
      {
	delete(_ra_objects[i]);
	_ra_objects.erase(_ra_objects.begin() + i);
      }

  res = res + _ffs->ffsClose(inode);

  return res;
}

/**
 * Drop the page cache. As in 'echo 3 > /proc/sys/vm/drop_caches', here we do not
 * sync. sync must be called before to drop dirty data
 */
PpcValF VirtualFileSystem::vfsDropCache ()
{
  PpcValF res = {0, 0};

  res = res + _page_cache->drop();

  return res;
}
