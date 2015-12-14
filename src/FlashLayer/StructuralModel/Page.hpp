/**
 * \file Page.hpp
 * \brief Page class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * Page class header.
 *
 */

#ifndef PAGE_HPP
#define PAGE_HPP

#include <iostream>

#include "../../Globs/Common.hpp"

using namespace std;

/**
 * \class Page
 * \brief A flash memory page
 * This is an intern component of the flash layer. It is designed to be
 * encapsulated inside the top level component of the flash layer which
 * is represented by the FlashSystem class.
 * This class should never be instantiated "as it is", only for test
 * purposes.
 */
class Page
{
	public:
		Page();
		virtual ~Page(void);
		// Getters and setters
		pageState_t getState();
		void * getPriv();
    int setPriv(void *data);
    
		//Methods related to statistics
		void resetStats();

	
	private:
		pageState_t _state;					/**< \brief Page state */
		void *_priv; 								/**< \brief data structure used by 
																			flash management layer*/
		uint64_t _stats[NUMBER_OF_PAGE_STATS];

		void incrStat(pageStat_t stat);
    
  /** print info about the page on a stream */
	friend ostream& operator<<(ostream& os, const Page& p);
	friend class FlashSystem;
};

#endif /* PAGE_HPP */
