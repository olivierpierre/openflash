/**
 * \file Plane.cpp
 * \brief Plane class implementation
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/09
 *
 * Plane class implementation.
 */

#include "Plane.hpp"

/**
 * \fn Plane::Plane(int nbBlocks, int nbPagesPerBlock)
 * \brief Constructor
 * \param nbBlocks number of blocks in the plane
 * \param nbPagesPerBlock number of pages in each block in the plane
 */
Plane::Plane(int nbBlocks, int nbPagesPerBlock)
{
	for(int i=0; i<nbBlocks; i++)
		_blocks.push_back(Block(nbPagesPerBlock));
		
	resetStats();
}

/**
 * \fn virtual Plane::~Plane()
 * \brief Destructor
 */
Plane::~Plane(){}

/**
 * \fn void Plane::resetStats();
 * \brief Resets all stats and counters to 0
 */
void Plane::resetStats()
{
  for(int i=0; i<NUMBER_OF_PLANE_STATS; i++)
    _stats[i] = 0;
	for(int i=0; i<(int)_blocks.size(); i++)
		_blocks[i].resetStats();
}

void
Plane::incrStat(planeStat_t stat)
{
  _stats[stat]++;
}

/**
 * \fn ostream& operator<<(ostream& os, Plane& p)
 * \brief Print plane info on a stream
 */
ostream& operator<<(ostream& os, Plane& p)
{
    os << "Plane with " << p._blocks.size() << " blocks";
		
    return os;
}
