/**
 * \file Channel.hpp
 * \brief Channel class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/12
 *
 * Channel class header.
 *
 */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>

#include "Lun.hpp"
#include "../../Globs/Common.hpp"

/**
 * \class Channel
 * \brief A channel linking together several LUNS
 * 
 * This is an intern component of the flash layer. It is designed to be
 * encapsulated inside the top level component of the flash layer which
 * is represented by the FlashSystem class.
 * This class should never be instantiated "as it is", only for test
 * purposes.
 */
class Channel
{
	public:
		Channel(	int nbLuns, int nbPlanesPerLun,
							int nbBlocksPerPlane, int nbPagesPerBlock);
		virtual ~Channel();
    
		// stats
		void resetStats();
		void incrStat(channelStat_t stat);
			
	private:
		vector<Lun> _luns;    /**< flash chips composing the channel */
		uint64_t _stats[NUMBER_OF_CHANNEL_STATS];
		
	friend ostream& operator<<(ostream& os, Channel& c); /**< print on stream */
	friend class FlashSystem;
};

#endif /* CHANNEL_HPP */
