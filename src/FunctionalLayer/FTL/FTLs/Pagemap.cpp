/*
 * Pagemap.cpp
 *
 *  Created on: 9 avr. 2014
 *      Author: pierre
 */

#include "Pagemap.hpp"

/**
 * Constructor : Pagemap FTL initialization
 */
Pagemap::Pagemap () : FlashTranslationLayer(FlashSystem::getInstance())
{
  uint32_t nb_pages_total = _f->getStructuralParameter(SP_PAGE_NUM);
  int nb_pages_per_block = _f->getStructuralParameter(SP_PAGES_PER_BLOCK);
  uint32_t nb_blocks = _f->getStructuralParameter(SP_BLOCK_NUM);

#ifdef VERBOSE
  cout << "[PM] init PM" << endl;
#endif /* VERBOSE */

  /* init mapping table */
  for(uint32_t i=0; i< nb_pages_total; i++)
  {
    pm_mapping_table_entry entry = {0xFFFFFFFF, PM_FREE};
    _mapping_table.push_back(entry);
  }

  /* init blocks metadata */
  for(uint32_t i=0; i< nb_blocks; i++)
  {
    pm_block block = {0, nb_pages_per_block};
    _blocks.push_back(block);
  }

  /* set gc threshold */
  _gc_threshold = Param::getInstance()->getInt("functional_model.ftl.pagemap.gc_threshold");

  _next_free_page = 0;
}

Pagemap::~Pagemap () {}

/**
 * Read operation
 */
PpcValF2 Pagemap::ftlRead (Address address)
{
  PpcValF2 res = {0, 0};
#ifdef VERBOSE
  cout << "[PM] Read @" << address << endl;
#endif /* VERBOSE */

  /**
   * compute logical page number, and look in the mapping table for the
   * corresponding physical page number
   */
  uint32_t logical_page_number = addressToPageIndex(address);
  uint32_t physical_page_number = findInMappingTable(logical_page_number);

  if(physical_page_number == 0xFFFFFFFF)
    ERROR("PM : Cant find page " + nts(logical_page_number) + " to read in mapping table. Was it previously written ?");

  if(_mapping_table[physical_page_number].state != PM_VALID)
    ERROR("PM : trying to read free or invalid page");

  /**
   * convert the physical page number to an address then actually
   * read the page on flash
   */
  Address physical_page_address = pageIndexToAddress(physical_page_number);
  res = res + flashLegacyRead(physical_page_address);

  return res;
}

/**
 * Write operation
 */
PpcValF2 Pagemap::ftlWrite (Address address)
{
  PpcValF2 res = {0, 0};
  //TODO
#ifdef VERBOSE
  cout << "[PM] Write @" << address << endl;
#endif /* VERBOSE */

/* if we are low on free space GC one block */
  if(countUsedBlocks() <= (uint32_t)_gc_threshold)
    res = res + gcRecycleOneBlock();

  /* compute logical page number */
  uint32_t logical_page_number = addressToPageIndex(address);

  /* flashWriteWrapper will take care of finding a place to write that
   * page and update the mapping table */
  res = res + flashWriteWrapper(logical_page_number);

  return res;
}

/**
 * TODO see how trim is really implemented, this is just an
 * intuitive and stupid implementation
 */
PpcValF2 Pagemap::ftlTrim (Address address)
{
  PpcValF2 res = {0, 0};
#ifdef VERBOSE
  cout << "[PM] Trim @" << address << endl;
#endif /* VERBOSE */

  /**
   * compute logical page number, and look in the mapping table for the
   * corresponding physical page number
   */
  uint32_t logical_page_number = addressToPageIndex(address);
  uint32_t physical_page_number = findInMappingTable(logical_page_number);

  if(physical_page_number == 0xFFFFFFFF)
    ERROR("PM : Cant find page " + nts(logical_page_number) + " to trim in mapping table. Was it previously written ?");

  /* invalidate the corresponding page */
  invalidatePhysicalPage(physical_page_number);

  return res;
}

/**
 * Garbage collector : launched when we are low on free space, used to
 * recycle (erase) 1 block
 */
PpcValF2 Pagemap::gcRecycleOneBlock ()
{
  PpcValF2 res = {0, 0};
  uint32_t victim_block_index = 0xFFFFFFFF;
  int victim_block_invalid_pages = 0;

  /* find the block with the most important number of invalid pages : 
   * it is the victim block
   */
  for(int i=0; i<(int)_blocks.size(); i++)
    if(_blocks[i].invalid_pages > victim_block_invalid_pages)
    {
      victim_block_index = i;
      victim_block_invalid_pages = _blocks[i].invalid_pages;
    }

  if(victim_block_index == 0xFFFFFFFF)
    ERROR("Could not find any block to gc they are all fully valid !!!");

  //copy all still valid pages of the victim block to another location
  for(int i=0; i<(int)PAGES_PER_BLOCK; i++)
  {
    uint32_t physical_page_number = victim_block_index*PAGES_PER_BLOCK + i;

    if(_mapping_table[physical_page_number].state == PM_VALID)  /* if the page is still valid */
    {
      uint32_t logical_page_number = _mapping_table[physical_page_number].lpn;
      Address addr = pageIndexToAddress(physical_page_number);
      /* read the stil valid page */
      res = res + flashLegacyRead(addr);

      /* the write it somewhere else, update mapping table */
      res = res + flashWriteWrapper(logical_page_number);
    }
  }

  //erase the block, update block and mapping table metadata
  Address addr = pageIndexToAddress(victim_block_index*PAGES_PER_BLOCK);
  res = res + flashLegacyErase(addr);

  /* update mapping table & block metadata */
  for(int i=0; i<(int)PAGES_PER_BLOCK; i++)
  {
    _mapping_table[victim_block_index*PAGES_PER_BLOCK + i].state = PM_FREE;
    _mapping_table[victim_block_index*PAGES_PER_BLOCK + i].lpn = 0xFFFFFFFF;
  }
  _blocks[victim_block_index].free_pages = PAGES_PER_BLOCK;
  _blocks[victim_block_index].invalid_pages = 0;


  return res;
}

/**
 * Convert an address object (channel, lun, plane, block, page) to a page 
 * number
 */
uint32_t Pagemap::addressToPageIndex (Address address)
{
  uint32_t res = 0;

  res = address.getPage() +
	address.getBlock() * PAGES_PER_BLOCK +
	address.getPlane() * PAGES_PER_BLOCK*BLOCKS_PER_PLANE +
	address.getLun() * PAGES_PER_BLOCK*BLOCKS_PER_PLANE*PLANES_PER_LUN +
	address.getChannel() * PAGES_PER_BLOCK*BLOCKS_PER_PLANE*PLANES_PER_LUN*LUNS_PER_CHANNEL;

  return res;
}

/**
 * Convert a page number to an address object
 */
Address Pagemap::pageIndexToAddress (uint32_t page_index)
{
  int page, block, plane, lun, channel;
  page = page_index % PAGES_PER_BLOCK;
  block = (page_index / PAGES_PER_BLOCK) % BLOCKS_PER_PLANE;
  plane = ((page_index / PAGES_PER_BLOCK)/BLOCKS_PER_PLANE) % PLANES_PER_LUN;
  lun = (((page_index / PAGES_PER_BLOCK)/BLOCKS_PER_PLANE)/PLANES_PER_LUN) % LUNS_PER_CHANNEL;
  channel = (((page_index / PAGES_PER_BLOCK)/BLOCKS_PER_PLANE)/PLANES_PER_LUN)/LUNS_PER_CHANNEL;

  return (Address(channel, lun, plane, block, page));
}

/**
 * Find a logical page in the mapping table. return the index if found, which is also
 * the corresponding physical page number
 * returns 0xFFFFFFFF if not found
 */
uint32_t Pagemap::findInMappingTable (uint32_t logical_page_number)
{
  for(uint32_t i=0; i<_mapping_table.size(); i++)
    if(_mapping_table[i].lpn == logical_page_number && _mapping_table[i].state == PM_VALID)
      return i;

  return 0xFFFFFFFF;
}

/**
 * Invalidate a given page in the mapping table
 */
int Pagemap::invalidatePhysicalPage (uint32_t physical_page_index)
{
  if(_mapping_table[physical_page_index].state != PM_VALID)
    ERROR("PM : trying to invalidate page " + nts(physical_page_index) + " which is invalid or free");

  _mapping_table[physical_page_index].state = PM_INVALID;
  _blocks[physical_page_index/PAGES_PER_BLOCK].invalid_pages++;

  return 0;
}

/**
 * Count the number of non free blocks, used to determine if the amount
 * of free space is low to launch GC
 */
uint32_t Pagemap::countUsedBlocks ()
{
  uint32_t res = 0;

  for(uint32_t i=0; i<_blocks.size(); i++)
    if(_blocks[i].free_pages == (int)PAGES_PER_BLOCK)
      res++;

  return res;
}

/**
 * Write a logical page on flash
 * 1. invalidate potential old versions
 * 2.Perform checks on the state of the page being written
 * 3. update mapping table and blocks metadata
 * 4. write
 * 5. update next free page pointer
 */
PpcValF2 Pagemap::flashWriteWrapper (uint32_t logical_page_number)
{
  PpcValF2 res = {0, 0};

  /* invalidate old version if necessary */
  uint32_t old_physical_page = findInMappingTable(logical_page_number);
  if(old_physical_page != 0xFFFFFFFF)
    invalidatePhysicalPage(old_physical_page);

  if(_mapping_table[_next_free_page].state != PM_FREE)
    ERROR("Trying to write on non free page");

  /* update mapping table */
  _mapping_table[_next_free_page].lpn = logical_page_number;
  _mapping_table[_next_free_page].state = PM_VALID;

  /* write on flash @ next_free_page */
  Address physical_address = pageIndexToAddress(_next_free_page);
  res = res + flashLegacyWrite(physical_address);

  /* update block metadata */
  _blocks[_next_free_page / PAGES_PER_BLOCK].free_pages--;

  /* update next free page */
  updateNextFreePage();

  return res;
}

/**
 * update the next_free_page pointer : this variable always points on
 * the next page to be written on flash
 */
int Pagemap::updateNextFreePage()
{
  _next_free_page++;

  if((_next_free_page % PAGES_PER_BLOCK) == 0)
  {
    // find a new free block because we have crossed th current block 
    // boundaries
    for(int i=0; i<(int)_blocks.size(); i++)
      if(_blocks[i].free_pages == (int)PAGES_PER_BLOCK)
      {
	_next_free_page = i*PAGES_PER_BLOCK;
#ifdef VERBOSE
  cout << "PM : next free page : " << _next_free_page << endl;
#endif /* VERBOSE */
	return 0;
      }

    ERROR("Ran out of free space ...");
  }

#ifdef VERBOSE
  cout << "PM : next free page : " << _next_free_page << endl;
#endif /* VERBOSE */

  return 0;
}
