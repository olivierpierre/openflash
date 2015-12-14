/*
 * Pagemap.hpp
 *
 *  Created on: 9 avr. 2014
 *      Author: pierre
 */

#ifndef PAGEMAP_HPP
#define PAGEMAP_HPP

#include "../FlashTranslationLayer.hpp"

/**
 * State of a physical flash page
 */
typedef enum
{
  PM_VALID,
  PM_INVALID,
  PM_FREE
} pm_page_state;

/**
 * Metadata for 1 physical flash page
 */
typedef struct
{
  uint32_t 		lpn;			// logical page number
  pm_page_state 	state;			// state of the page
} pm_mapping_table_entry;

/**
 * Metatdata for 1 physical flash block
 */
typedef struct
{
  int			invalid_pages;		// number of invalid pages
  int 			free_pages;		// number of free pages
}pm_block;

/**
 * The pagemap FTL class
 */
class Pagemap : public FlashTranslationLayer
{
public:
  Pagemap ();
  virtual ~Pagemap ();

  PpcValF2 ftlRead(Address address);       // Basic operations : read
  PpcValF2 ftlWrite(Address address);      // write
  PpcValF2 ftlTrim(Address address);       // trim



private:
  int 					_gc_threshold;	/* number of free blocks to trigger gc */
  vector<pm_mapping_table_entry>	_mapping_table;	/* mapping table */
  vector<pm_block>			_blocks;	/* metadata on physical blocks */
  uint32_t				_next_free_page;	/* pointer on next free page to write */

  PpcValF2 gcRecycleOneBlock();	// gc execution

/* see .cpp for more info on those functions */
  uint32_t addressToPageIndex(Address address);
  Address pageIndexToAddress(uint32_t page_index);
  uint32_t findInMappingTable(uint32_t logical_page_number);
  int invalidatePhysicalPage(uint32_t physical_page_index);
  uint32_t countUsedBlocks();
  PpcValF2 flashWriteWrapper(uint32_t logical_page_number);
  int updateNextFreePage();

};

#endif /* PAGEMAP_HPP */
