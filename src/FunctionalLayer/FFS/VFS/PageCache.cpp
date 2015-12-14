#include "PageCache.hpp"

#include <iostream>

#include "../FlashFileSystem.hpp"

using namespace std;

PageCache *PageCache::_singleton = NULL;

LinuxPage::LinuxPage (uint32_t inode_num, uint32_t page_index) :
    _inode_num(inode_num), _page_index(page_index), _readahead_flag(false) {}

PageCache* PageCache::getInstance ()
{
  if(_singleton == NULL)
    _singleton = new PageCache(Param::getInstance()->getInt("functional_model.vfs.page_cache_size_bytes"));
  return _singleton;
}

void PageCache::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

PageCache::PageCache (uint64_t size_bytes)
{
  _max_size_pages = size_bytes / LINUX_PAGE_SIZE;
}

/**
 * If the page is present in the page cache, return a pointer to the page object
 * return NULL otherwise
 */
LinuxPage *PageCache::findInPageCache (uint32_t inode, uint32_t page_index)
{
  for(int i=0; i<(int)_page_cache.size(); i++)
    if(_page_cache[i]->getInodeNum() == inode && _page_cache[i]->getPageIndex() == page_index)
      return _page_cache[i];
  return NULL;
}

/**
 * Insert a new page in the page cache
 */
int PageCache::insertInPageCache (uint32_t inode, uint32_t page_index)
{
  if(_page_cache.size() == _max_size_pages)
    evictPageFromPageCache();

  LinuxPage *p = new LinuxPage(inode, page_index);
  _page_cache.push_back(p);

  // debug
#ifndef NDEBUG
  sanityCheck(inode, page_index);
#endif /* NDEBUG */

  return 0;
}

int PageCache::setLruAccessed (uint32_t inode, uint32_t page_index)
{
  int index = getIndexInPageCache(inode, page_index);
//  bool ra_flag = _page_cache[index]->getReadAheadFlag();
//
//  if(index == -1)
//    ERROR("trying to set lru accessed for a page non present in the page cache");
//
//  internalRemovePC(index);
//
//  LinuxPage *p = new LinuxPage(inode, page_index);
//  if(ra_flag)
//    p->setReadAheadFlag();	// this is really ugly fix it !!!!
//  _page_cache.push_back(p);

  //TODO this is not a LRU at all !!!!
  swap(_page_cache[index], _page_cache[_page_cache.size()-1]);

  return 0;
}

/**
 * Evict the LRU page from the PC
 */
int PageCache::evictPageFromPageCache ()
{
  // evict LRU page (TODO writeback if needed)
  if(_page_cache.empty())
    ERROR("Asking to remove a page from the PC while PC is empty");

#ifdef VERBOSE
  cout << "[PC] evict ino" << _page_cache[0]->getInodeNum() <<
      " page " << _page_cache[0]->getPageIndex() << endl;
#endif /* VERBOSE */

  internalRemovePC(0);
  return 0;
}

/**
 * Internal
 * Return the index of the page in the _page_cache vector.
 * if not found returns -1
 */
int PageCache::getIndexInPageCache (uint32_t inode, uint32_t page_index)
{
  for(int i=0; i<(int)_page_cache.size(); i++)
    if(_page_cache[i]->getInodeNum() == inode && _page_cache[i]->getPageIndex() == page_index)
      return i;
  return -1;
}

/**
 * Return a vector containing pointers to all pages in PC for a given page
 * Pages are ordered based on their index in the file
 */
vector<LinuxPage *> PageCache::getOrderedPagesForFile (uint32_t inode)
{
  vector<LinuxPage *> res;

  for(int i=0; i<(int)_page_cache.size(); i++)
    if(_page_cache[i]->getInodeNum() == inode)
    {
      bool inserted = false;
      for(int j=0; j<(int)res.size(); j++)
	if(_page_cache[i]->getPageIndex() < res[j]->getPageIndex())
	{
	  res.insert(res.begin()+j, _page_cache[i]);
	  inserted = true;
	  break;
	}
      if(!inserted)
	res.push_back(_page_cache[i]);
    }

  return res;
}

PageCache::~PageCache ()
{
  for(int i=0; i<(int)_page_cache.size(); i++)
    delete _page_cache[i];
}

PpcValF PageCache::drop()
{
  PpcValF res = {0, 0};

  for(int i=0; i<(int)_page_cache.size(); i++)
    delete _page_cache[i];

  _page_cache.clear();

  return res;
}

/**
 * Delete a page form the PC and free corresponding memory
 */
int PageCache::internalRemovePC (int index_in_pc_vector)
{

  delete _page_cache[index_in_pc_vector];

  _page_cache.erase(_page_cache.begin() + index_in_pc_vector);

  return 0;
}

/**
 * Check that an index we just inserted in the page cache is only present one time
 */
int PageCache::sanityCheck (uint32_t inode, uint32_t inserted_index)
{
  int count = 0;

  for(int i=0; i<(int)_page_cache.size(); i++)
    if(inode == _page_cache[i]->getInodeNum() && inserted_index == _page_cache[i]->getPageIndex())
      count++;

  if(count != 1)
  {
    ERROR("count for inserted ino#" + nts(inode) + " page "+ nts(inserted_index) + " == " + nts(count));
    return -1;
  }
  return 0;
}
