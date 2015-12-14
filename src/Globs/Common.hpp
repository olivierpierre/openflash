/**
 * \file Common.hpp
 * \brief Common header file for all source files
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/12
 */

#ifndef COMMON_HPP
#define COMMON_HPP

#include "../Globs/Common.hpp"

#include <stdint.h>

#include "../Globs/ErrorManager.hpp"
#include "../Globs/Param.hpp"
#include "../TimeEvents/GlobalTime.hpp"

//#define VERBOSE
//#define NDEBUG

/* number to string */
#define nts(x) static_cast<ostringstream*>( &(ostringstream() << (x)) )->str()

#define OF_MAJOR_REVISION	42
#define OF_COMPILE_DATE 	__DATE__ "." __TIME__

#ifndef NULL
#define NULL   ((void *) 0)
#endif

/**
 * \enum pageState_t
 * \brief The state of one flash page
 * It can be : 1) FREE page means we can write in that page ; 2) 
 * OCUPPIED means the page contains data and cannot be written without
 * being erased
 */
typedef enum
{
	PAGE_FREE,
	PAGE_OCCUPIED
} pageState_t;

/**
 * \enum blockState_t
 * \brief The state of one flash block
 * It can be : 1) BLOCK_FREE : full of free pages ; 2)
 * BLOCK_OCCUPIED : at least one page is written : 3) BLOCK_FULL :
 * compeltely written
 */
typedef enum
{
	BLOCK_FREE,
	BLOCK_OCCUPIED,
	BLOCK_FULL
} blockState_t;

/**
 * \enum structuralParam_t
 * \brief Identifiers for structural parameers
 */
typedef enum
{
  SP_PAGES_PER_BLOCK=0,         //!< SP_PAGES_PER_BLOCK
  SP_BLOCKS_PER_PLANE,        //!< SP_BLOCKS_PER_PLANE
  SP_PLANES_PER_LUN,          //!< SP_PLANES_PER_LUN
  SP_LUNS_PER_CHANNEL,         //!< SP_LUN_PER_CHANNEL
  SP_PAGE_NUM,                //!< SP_PAGE_NUM
  SP_BLOCK_NUM,               //!< SP_BLOCK_NUM
  SP_PLANE_NUM,               //!< SP_PLANE_NUM
  SP_LUN_NUM,                 //!< SP_LUN_NUM
  SP_CHANNEL_NUM,             //!< SP_CHANNEL_NUM
  SP_PAGE_SIZE_BYTE,
  SP_BLOCK_SIZE_BYTE,
  NUMBER_OF_STRUCTURAL_PARAMS //!< NUMBER_OF_STRUCTURAL_PARAMS
} structuralParam_t;

/**
 * Statistics on each level of the flash storage structure hierarchy
 */

typedef enum
{
  MULTI_CHANNEL_NUM,        /* number of multi channels operations performed */
  NUMBER_OF_FLASHSYSTEM_STATS //!< NUMBER_OF_FLASHSYSTEM_STATS
}flashSystemStat_t;

typedef enum
{
  MULTI_LUN_NUM,          /* number of multi lun operation performed in the channel */
  NUMBER_OF_CHANNEL_STATS
}channelStat_t;

typedef enum
{
  MULTIPLANE_READ_NUM,     /* number of mpr ops received by the lun */
  MULTIPLANE_WRITE_NUM,     /* number of mpw received */
  MULTIPLANE_ERASE_NUM,     /* number of mpe received */
  MPCOPYBACK_NUM,           /* number of multi plane copy back operations received */
  MPCACHEREAD_NUM,          /* number of multi plane cache read ops received */
  MPCACHEWRITE_NUM,         /* number of multi plane cache write ops received */
  NUMBER_OF_LUN_STATS
}lunStat_t;

typedef enum
{
  CACHE_READ_NUM_PLANE=0,     /* number of cr ops received by the plane */
  CACHE_WRITE_NUM_PLANE,      /* number of cw ops received by the plane */
  COPYBACK_NUM,               /* Number of copyback ops received */
  NUMBER_OF_PLANE_STATS
}planeStat_t;

typedef enum
{
  LEGACY_ERASE_NUM=0,
  MULTIPLANE_ERASE_BLOCK,     /* Erased during multi plane erase operation */
  NUMBER_OF_BLOCK_STATS
}blockStat_t;

typedef enum
{
  LEGACY_READ_NUM=0,        /* Number of times the page was read in legacy mode */
  LEGACY_WRITE_NUM,       /*                              written */
  CACHE_READ_NUM_PAGE,         /* Number of times the page was read in cache read mode */
  CACHE_WRITE_NUM_PAGE,
  COPYBACK_SRC,           /* read as source for copyback */
  COPYBACK_TARGET,        /* written as target for copyback */
  MULTIPLANE_READ_PAGE,     /* read as part of multi plane read operation */
  MULTIPLANE_WRITE_PAGE,  /* written during multi plane operation */
  MPCOPYBACK_SRC,         /* read as source for a multi plane copy back operation */
  MPCOPYBACK_TRGT,        /* written as target for a copyback operation */
  MP_CACHE_READ,          /* read during multi plane cache read */
  MP_CACHE_WRITE,         /* written during multi plane cache write */
  NUMBER_OF_PAGE_STATS
}pageStat_t;

/* a par/seq for multi lun operations */
typedef enum
{
  PAR,
  SEQ
} multiLunSubJobType_t;

/* a set of ordered par/seq for multi lun operations */
typedef struct
{
  multiLunSubJobType_t type;
  double time;
} multiLunSubJob_t;

/* some useful macros, only valid after initialization */
#define PAGE_NUM		(FlashSystem::getInstance()->getStructuralParameter(SP_PAGE_NUM))
#define BLOCK_NUM		(FlashSystem::getInstance()->getStructuralParameter(SP_BLOCK_NUM))
#define PLANE_NUM		(FlashSystem::getInstance()->getStructuralParameter(SP_PLANE_NUM))
#define LUN_NUM			(FlashSystem::getInstance()->getStructuralParameter(SP_LUN_NUM))
#define CHANNEL_NUM		(FlashSystem::getInstance()->getStructuralParameter(SP_CHANNEL_NUM))

#define PAGES_PER_BLOCK		(FlashSystem::getInstance()->getStructuralParameter(SP_PAGES_PER_BLOCK))
#define BLOCKS_PER_PLANE	(FlashSystem::getInstance()->getStructuralParameter(SP_BLOCKS_PER_PLANE))
#define PLANES_PER_LUN		(FlashSystem::getInstance()->getStructuralParameter(SP_PLANES_PER_LUN))
#define LUNS_PER_CHANNEL	(FlashSystem::getInstance()->getStructuralParameter(SP_LUNS_PER_CHANNEL))

#define PAGE_SIZE_BYTE		(FlashSystem::getInstance()->getStructuralParameter(SP_PAGE_SIZE_BYTE))
#define BLOCK_SIZE_BYTE		((PAGE_SIZE_BYTE)*(PAGES_PER_BLOCK))
#define PLANE_SIZE_BYTE		((BLOCK_SIZE_BYTE)*(BLOCKS_PER_PLANE))
#define LUN_SIZE_BYTE		((PLANE_SIZE_BYTE)*(PLANES_PER_LUN))
#define CHANNEL_SIZE_BYTE	((LUN_SIZE_BYTE)*(LUNS_PER_CHANNEL))

#define SECTOR_SIZE_BYTES	(512)
#define PAGE_SIZE_SECTORS	((PAGE_SIZE_BYTE) / (SECTOR_SIZE_BYTES))
#define BLOCK_SIZE_SECTORS	((PAGE_SIZE_SECTORS) * (PAGES_PER_BLOCK))
#define PLANE_SIZE_SECTORS	((BLOCK_SIZE_SECTORS) * (BLOCKS_PER_PLANE))
#define LUN_SIZE_SECTORS	((PLANE_SIZE_SECTORS) * (PLANES_PER_LUN))
#define CHANNEL_SIZE_SECTORS	((LUN_SIZE_SECTORS) * (LUNS_PER_CHANNEL))

#define TIME_NOW		(TimeMgr::getInstance()->getTime())

#define ERROR(x)		ErrorManager::getInstance()->error(x)
#define ERROR2(x,y,z)		ErrorManager::getInstance()->error(x, y, z)
#define WARNING(x)		ErrorManager::getInstance()->warning(x)

#endif /* COMMON_HPP */
