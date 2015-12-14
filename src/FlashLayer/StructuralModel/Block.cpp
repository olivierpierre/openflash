/**
 * \file Block.cpp
 * \brief Block class implementation
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * Block class implementation.
 *
 */

#include "Block.hpp"
#include <iostream>

using namespace std;
/**
 * \fn Block::Block(int nbPages)
 * \param nbPages Number of flash pages contained in the block
 * \brief Block constructor
 */
Block::Block(int nbPages)
{
	_pages.resize(0);
	for(int i=0; i<nbPages; i++)
	  _pages.push_back(Page());
	_lastPageWrittenOffset = -1;	/* Because 0 must be the first page 
																 * written on a clean block */
}

/**
 * \fn Block::~Block()
 * \brief Block destructor
 */
Block::~Block(){}

/**
 * \fn void Block::resetStats()
 * \brief Resets all stats and counters to zero
 */
void Block::resetStats()
{
  for(int i=0; i<NUMBER_OF_BLOCK_STATS; i++)
    _stats[i] = 0;

	/** \note we do not reset _lastPageWrittenOffset because at the end 
	 * of the warmup, there is a reset stats but the simulation 
	 * continues afterwards ! */

	for(int i=0; i<(int)_pages.size(); i++)
		_pages[i].resetStats();
}

/**
 * \fn blockState_t Block::getState()
 * \brief Returns the state of the block
 */
blockState_t Block::getState()
{
  int occupiedPages = 0;

  for(int i=0; i<(int)_pages.size(); i++)
    if(_pages[i].getState() == PAGE_OCCUPIED)
      occupiedPages++;

  if(occupiedPages == 0)
    return BLOCK_FREE;
  else if (occupiedPages == (int)_pages.size())
    return BLOCK_FULL;
  else return BLOCK_OCCUPIED;
}

/**
 * \fn void Block::incrStat(blockStat_t stat)
 * \brief Increment a statistic counter related to the block
 */
void Block::incrStat(blockStat_t stat)
{
  _stats[stat]++;
}
