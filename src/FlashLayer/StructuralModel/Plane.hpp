/**
 * \file Plane.hpp
 * \brief Plane class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/09
 *
 * Plane class header.
 *
 */

#ifndef PLANE_HPP
#define PLANE_HPP

#include <vector>

#include "../../Globs/Common.hpp"
#include "Block.hpp"
#include "../Address.hpp"

/**
 * \class Plane
 * \brief A flash memory plane
 * This is an intern component of the flash layer. It is designed to be
 * encapsulated inside the top level component of the flash layer which
 * is represented by the FlashSystem class.
 * This class should never be instantiated "as it is", only for test
 * purposes.
 */
class Plane
{
	public:
		Plane(int nbBlocks, int nbPagesPerBlock);
		virtual ~Plane();
		//Getters / setters :
		
		// stats
		void resetStats();
		

	private:
		vector<Block> _blocks;  /**< blocks composing the plane */
		uint64_t _stats[NUMBER_OF_PLANE_STATS];
		
		void incrStat(planeStat_t stat);

  /** print info about the plane on a stream */
	friend ostream& operator<<(ostream& os, Plane& p);
	friend class FlashSystem;
};

#endif /* PLANE_HPP */
