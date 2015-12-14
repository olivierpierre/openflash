/**
 * \file Channel.cpp
 * \brief Channel class implementation
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/12
 *
 * Channel class implementation.
 */

#include "Channel.hpp"

/**
 * \fn Channel::Channel(int nbChips, int nbDiesPerChip, int nbPlanesPerLun,
 *							int nbBlocksPerPlane, int nbPagesPerBlock)
 * \brief Constructor
 */
Channel::Channel(int nbLuns, int nbPlanesPerLun,
							int nbBlocksPerPlane, int nbPagesPerBlock)
{
	for(int i=0; i<nbLuns; i++)
		_luns.push_back(	Lun(nbPlanesPerLun,
											nbBlocksPerPlane, nbPagesPerBlock));
	resetStats();
}

Channel::~Channel(){}

/**
 * \fn void Channel::resetStats()
 * \brief reset all stats and counters to 0
 */
void Channel::resetStats()
{
  for(int i=0; i<NUMBER_OF_CHANNEL_STATS; i++)
    _stats[i] = 0;
	for(int i=0; i<(int)_luns.size(); i++)
		_luns[i].resetStats();
}

/**
 * void Channel::incrStat(channelStat_t stat)
 * \brief Increment a channel related statistic counter
 */
void Channel::incrStat(channelStat_t stat)
{
  _stats[stat]++;
}

/**
 * \fn ostream& operator<<(ostream& os, Channel& c)
 * \brief Print channel info on a stream
 */
ostream& operator<<(ostream& os, Channel& c)
{
    os << "Channel with " << c._luns.size() << " LUNs";
		
    return os;
}
