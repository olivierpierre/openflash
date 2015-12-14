/**
 * \file Block.hpp
 * \brief Block class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * Block class header, represents a flash memory block.
 *
 */

#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <vector>

#include "../../Globs/Common.hpp"
#include "Page.hpp"
#include "../Address.hpp"

using namespace std;

/**
 * \class Block
 * \brief A flash memory block
 * 
 * This is an intern component of the flash layer. It is designed to be
 * encapsulated inside the top level component of the flash layer which
 * is represented by the FlashSystem class.
 * This class should never be instantiated "as it is", only for test
 * purposes.
 */
class Block
{
	public:
		Block(int nbPages);
		virtual ~Block(void);
		
		// Getters / setters
		blockState_t getState();
		
		// Methods related to statistics
		void resetStats();
		
	private:
		vector<Page> _pages;					/**< \brief an array of Pages contained
																				in the block */
		// statistics
		int _lastPageWrittenOffset;		/**< \brief For checking if writes are
																				sequential within a block */
		uint64_t _stats[NUMBER_OF_BLOCK_STATS];
		
		void incrStat(blockStat_t stat);

	friend ostream& operator<<(ostream& os, Block& b);  /**< print on stream */
	friend class FlashSystem;
};

#endif /* BLOCK_HPP */
