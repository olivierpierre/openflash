/**
 * \file FlashSystem.hpp
 * \brief FlashSystem class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * FlashSystem class header.
 */

#ifndef FLASH_SYSTEM_HPP
#define FLASH_SYSTEM_HPP

#include <vector>

#include "../Globs/Common.hpp"
#include "Address.hpp"
#include "StructuralModel/Channel.hpp"

/**
 * \enum flashsystem_level_t
 * \brief Used with addressRangeCheck() to specify up to which level
 * the address is checked
 */
typedef enum
{
  PAGE_LEVEL,
  BLOCK_LEVEL,
  PLANE_LEVEL,
  LUN_LEVEL,
  CHANNEL_LEVEL
} flashsystem_level_t;

/**
 * \class FlashSystem
 * \brief 'Top level'
 * 
 * This is the flash layer "top level".
 * This is the only class that should be accessed by the flash 
 * management layer performing flash accesses during the simulation.
 * The other classes of the flash layer (page, block, plane, die, chip,
 * channel) are managed by the flash layer top level itself.
 */
class FlashSystem
{
public:
    static FlashSystem * getInstance();
    static void kill();
		
		// Getters / setters
    pageState_t getPageState(Address a);
    blockState_t getBlockState(Address a);
    int getBlockLastWrittenOffset(Address a);
    void *getPagePriv(Address a);
    int setPagePriv(Address a, void *data);
		
    uint32_t getStructuralParameter(structuralParam_t param);
    
    // supported commands
    /** todo */

    /* I/O sim state update methods*/
    int setWritten(Address pageAddress);
    void setErased(Address blockAddress);

    /* I/O sim stat update methods */
    void resetStats();
    void pageStatIncr(Address a, pageStat_t stat);
    void blockStatIncr(Address a, blockStat_t stat);
    void planeStatIncr(Address a, planeStat_t stat);
    void lunStatIncr(Address a, lunStat_t stat);
    void channelStatIncr(Address a, channelStat_t stat);
    void flashSystemStatIncr(flashSystemStat_t stat);

    uint64_t getPageStat(Address a, pageStat_t stat);
    uint64_t getBlockStat(Address a, blockStat_t stat);
    uint64_t getPlaneStat(Address a, planeStat_t stat);
    uint64_t getLunStat(Address a, lunStat_t stat);
    uint64_t getChannelStat(Address a, channelStat_t stat);
    uint64_t getFlashSystemStat(flashSystemStat_t stat);

    int getIoCycles();

    // Address range check
    bool addressRangeCheck(Address a, flashsystem_level_t level=PAGE_LEVEL);
			
	private:

    FlashSystem(  int nbChannels, int nbLunsPerChannel, int nbPlanesPerLun,
                      int nbBlocksPerPlane, int nbPagesPerBlock);
    ~FlashSystem();

		vector<Channel> _channels;  /**< channels of the flash system */
    uint64_t _stats[NUMBER_OF_FLASHSYSTEM_STATS];

    static FlashSystem *_singleton;

    int _ioCycles;      /* number of bus cycles to input / output a page */

		
  /** print info about flash system on a stream */
	friend ostream& operator<<(ostream& os, FlashSystem& f);
};

extern FlashSystem *flash;

#endif /* FLASH_SYSTEM_HPP */
