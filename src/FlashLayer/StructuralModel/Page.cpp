/**
 * \file Page.cpp
 * \brief Page class implementation
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * Page class implementation.
 */

#include "Page.hpp"

#include <iostream>

#include "../../Globs/Common.hpp"


using namespace std;

/**
 * \fn Page::Page()
 * \brief Constructor
 */
Page::Page()
{
	_state = PAGE_FREE;
	_priv = NULL;

	resetStats();
	
}

/**
 * \fn Page::~Page(void)
 * \brief Destructor
 */
Page::~Page(void){}

/**
 * \fn pageState_t Page::getState()
 * \brief Get the state of the page
 * \return the state of the page
 */
pageState_t Page::getState()
{
	return _state;
}

/**
 * \fn void * Page::getPriv()
 * \brief Get the private data pointer member of the page
 * \return the private data pointer member of the page
 */
void * Page::getPriv()
{
	return _priv;
}

/**
 * \fn void Page::resetStats()
 * \brief reset the statistics related to the page to 0 (i.e. read
 * and write counters)
 * Useful at the end of the warmup phase for example
 **/
void Page::resetStats()
{
  fill(_stats, _stats+NUMBER_OF_PAGE_STATS, 0);
}

void Page::incrStat(pageStat_t stat)
{
  _stats[stat]++;
}

/**
 * \fn ostream& operator<<(ostream& os, const Page& p)
 * \brief Print info about the page on a stream
 * \param os stream
 * \param p page
 **/
ostream& operator<<(ostream& os, const Page& p)
{
    os << "Page, state = " << p._state;
    return os;
}

/**
 * \fn int Page::setPriv(void *data)
 * \brief set the _priv member
 */
int Page::setPriv(void *data)
{
  if(data != NULL && _priv != NULL)
    ERROR("When setting priv data of a flash page, previous priv val is != NULL");

  _priv = data;
  return 0;
}
