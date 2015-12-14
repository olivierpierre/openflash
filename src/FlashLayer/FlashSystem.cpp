/**
 * \file FlashSystem.cpp
 * \brief FlashSystem class implementation
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * FlashSystem class implementation.
 */


#include "FlashSystem.hpp"
#include "../Globs/Common.hpp"


FlashSystem *FlashSystem::_singleton = NULL;

FlashSystem::FlashSystem(int nbChannels, int nbLunsPerChannel,
    int nbPlanesPerLun, int nbBlocksPerPlane, int nbPagesPerBlock)
{
	for(int i=0; i<nbChannels; i++)
		_channels.push_back(Channel(nbLunsPerChannel,
		    nbPlanesPerLun, nbBlocksPerPlane, nbPagesPerBlock));

	int channelWidthBits = Param::getInstance()->getInt("flash_layer.channel_width_bits");
	int pageSize = Param::getInstance()->getInt("flash_layer.page_size_bytes");
	int oobSize = Param::getInstance()->getInt("flash_layer.oob_size_bytes");

	_ioCycles = (pageSize + oobSize) / (8/channelWidthBits);

	resetStats();
}

FlashSystem::~FlashSystem(){}

void FlashSystem::resetStats()
{
  for(int i=0; i< NUMBER_OF_FLASHSYSTEM_STATS; i++)
    _stats[i] = 0;

	for(int i=0; i<(int)_channels.size(); i++)
		_channels[i].resetStats();
}


pageState_t FlashSystem::getPageState(Address a)
{
	if(!addressRangeCheck(a, PAGE_LEVEL))
		ERROR("Request for page state out of address range at FlashSystem level");
	return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
	    _blocks[a.getBlock()]._pages[a.getPage()]._state;
}

/**
 * \fn blockState_t FlashSystem::getBlockState(Address a)
 * \brief Return the state of the targeted block in the FlashSystem
 * \return the state of the targeted block in the FlashSystem
 * \param a address of the targeted block
 * Note that the page member of the address is not significant.
 */
blockState_t FlashSystem::getBlockState(Address a)
{
	if(!addressRangeCheck(a, BLOCK_LEVEL))
		ERROR("Request for block state out of address range at FlashSystem level");
	return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
	      _blocks[a.getBlock()].getState();
}

uint32_t FlashSystem::getStructuralParameter(structuralParam_t parameter)
{
  switch(parameter)
  {
  case SP_PAGES_PER_BLOCK:
    return _channels[0]._luns[0]._planes[0]._blocks[0]._pages.size();
    break;
  case SP_BLOCKS_PER_PLANE:
    return _channels[0]._luns[0]._planes[0]._blocks.size();
    break;
  case SP_PLANES_PER_LUN:
    return _channels[0]._luns[0]._planes.size();
    break;
  case SP_LUNS_PER_CHANNEL:
    return _channels[0]._luns.size();
    break;
  case SP_CHANNEL_NUM:
    return _channels.size();
    break;
  case SP_LUN_NUM:
    return _channels.size()*_channels[0]._luns.size();
    break;
  case SP_PLANE_NUM:
    return _channels.size()*_channels[0]._luns.size()*_channels[0]._luns[0]._planes.size();
    break;
  case SP_BLOCK_NUM:
    return _channels.size()*_channels[0]._luns.size()*_channels[0]._luns[0]._planes.size()*
	_channels[0]._luns[0]._planes[0]._blocks.size();
    break;
  case SP_PAGE_NUM:
    return _channels.size()*_channels[0]._luns.size()*_channels[0]._luns[0]._planes.size()*
	_channels[0]._luns[0]._planes[0]._blocks.size()*
	_channels[0]._luns[0]._planes[0]._blocks[0]._pages.size();
    break;
  case SP_PAGE_SIZE_BYTE:
    return Param::getInstance()->getInt("flash_layer.page_size_bytes");
    break;
  case SP_BLOCK_SIZE_BYTE:
    return getStructuralParameter(SP_PAGES_PER_BLOCK) * getStructuralParameter(SP_PAGE_SIZE_BYTE);
    break;
  default:
    ERROR("Request for wrong structural parameter");
  }

  return 0xFFFFFFFF;
}

int FlashSystem::setWritten(Address pageAddress)
{
  if(getPageState(pageAddress) != PAGE_FREE)
  {
    ERROR("write in non free page @" + pageAddress.str());
    return -1;
  }

  int lastOffset = getBlockLastWrittenOffset(pageAddress);
  if( lastOffset != pageAddress.getPage()-1)
  {
    ERROR("non seq write @ " + pageAddress.str() +
        ", previous written offset: " + nts(lastOffset) + "");
    return -2;
  }

  /* Checks have already been perfomed */
  _channels[pageAddress.getChannel()]._luns[pageAddress.getLun()].
    _planes[pageAddress.getPlane()]._blocks[pageAddress.getBlock()].
    _pages[pageAddress.getPage()]._state = PAGE_OCCUPIED;

  _channels[pageAddress.getChannel()]._luns[pageAddress.getLun()].
    _planes[pageAddress.getPlane()]._blocks[pageAddress.getBlock()].
    _lastPageWrittenOffset = pageAddress.getPage();

  return 0;
}

void FlashSystem::setErased(Address blockAddress)
{
  /* Checks have already been perfomed */
  for(int i=0; i<(int)(getStructuralParameter(SP_PAGES_PER_BLOCK)); i++)
  _channels[blockAddress.getChannel()]._luns[blockAddress.getLun()].
      _planes[blockAddress.getPlane()]._blocks[blockAddress.getBlock()].
      _pages[i]._state = PAGE_FREE;

  _channels[blockAddress.getChannel()]._luns[blockAddress.getLun()].
      _planes[blockAddress.getPlane()]._blocks[blockAddress.getBlock()].
      _lastPageWrittenOffset = -1;
}

uint64_t FlashSystem::getPageStat(Address a, pageStat_t stat)
{
  return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
      _blocks[a.getBlock()]._pages[a.getPage()]._stats[stat];
}

uint64_t FlashSystem::getBlockStat(Address a, blockStat_t stat)
{
  return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
      _blocks[a.getBlock()]._stats[stat];
}

uint64_t FlashSystem::getPlaneStat(Address a, planeStat_t stat)
{
  return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()]
                                                             ._stats[stat];
}

uint64_t FlashSystem::getLunStat(Address a, lunStat_t stat)
{
  return _channels[a.getChannel()]._luns[a.getLun()]._stats[stat];
}

uint64_t FlashSystem::getChannelStat(Address a, channelStat_t stat)
{
  return _channels[a.getChannel()]._stats[stat];
}

uint64_t FlashSystem::getFlashSystemStat(flashSystemStat_t stat)
{
  return _stats[stat];
}

/**
 * \fn ostream& operator<<(ostream& os, FlashSystem& f)
 * \brief Print info about the flash system on a stream
 * \param os stream
 * \param f FlashSystem object reference
 * 
 * Example :
 * ---------
 * \code
 * FlashSystem f(2,4,2,2,2048,64);
 * cout << f;
 * \endcode
 * Should print :
 * 
 **/
ostream& operator<<(ostream& os, FlashSystem& f)
{
  os << "FlashSystem(" << f.getStructuralParameter(SP_CHANNEL_NUM) << "," <<
      f.getStructuralParameter(SP_LUNS_PER_CHANNEL) << "," <<
      f.getStructuralParameter(SP_PLANES_PER_LUN) << "," <<
      f.getStructuralParameter(SP_BLOCKS_PER_PLANE) << "," <<
      f.getStructuralParameter(SP_PAGES_PER_BLOCK) << ")";
  return os;
}



/**
 * \fn void *FlashSystem::getPagePriv(Address a)
 * \brief return the _priv member of the targetted page
 */
void *FlashSystem::getPagePriv(Address a)
{
  if (!addressRangeCheck(a, PAGE_LEVEL))
      ERROR("Get page priv data failed address range check");
  return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
      _blocks[a.getBlock()]._pages[a.getPage()]._priv;
}

/**
 * \fn int FlashSystem::setPagePriv(Address a, void *data)
 * \brief set the _priv member of the targeted page to data
 */
int FlashSystem::setPagePriv(Address a, void *data)
{
  if (!addressRangeCheck(a, PAGE_LEVEL))
    ERROR("Set page priv data failed address range check");
  return _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
      _blocks[a.getBlock()]._pages[a.getPage()].setPriv(data);
}

/**
 * \fn bool FlashSystem::addressRangeCheck(Address a, flashsystem_level_t level)
 * \brief Perform an address range check of the address, according
 * to the flash system.
 * \param a the address to check
 * \param level up to which level the check is performed
 *
 * Example :
 * ---------
 * When performing a legacy page read operation the address must be
 * valid up to the page level (i.e. all the members of the address must
 * be included in the address range of the flashsystem). So when calling
 * addressRangeCheck up to the page level all the levels from channel
 * to page are checked.
 *
 * When performing a block erase operation the page member of the
 * address is not significant so the check is performed from the
 * channel memeber up to the block member. The page member is not
 * checked.
 */
bool FlashSystem::addressRangeCheck(Address a, flashsystem_level_t level)
{
  switch(level)
  {
    case PAGE_LEVEL:
      if(a.getPage() >= (int)getStructuralParameter(SP_PAGES_PER_BLOCK))
        return false;
        /* no break */

    case BLOCK_LEVEL:
      if(a.getBlock() >= (int)getStructuralParameter(SP_BLOCKS_PER_PLANE))
        return false;
        /* no break */

    case PLANE_LEVEL:
      if(a.getPlane() >= (int)getStructuralParameter(SP_PLANES_PER_LUN))
        return false;
        /* no break */

    case LUN_LEVEL:
      if(a.getLun() >= (int)getStructuralParameter(SP_LUNS_PER_CHANNEL))
        return false;
        /* no break */

    case CHANNEL_LEVEL:
      if(a.getChannel() >= (int)getStructuralParameter(SP_CHANNEL_NUM))
        return false;
        break;

    default:
      ERROR("Calling FlashSystem::addressRangeCheck on wrong level "
      "type");
      return false;
  }

  return true;
}

void FlashSystem::pageStatIncr(Address a, pageStat_t stat)
{
#ifdef DEBUG
  if(!addressRangeCheck(a), PAGE_LEVEL)
    em.error("Pagestatincr failed address range check");
#endif /* DEBUG */

  _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
    _blocks[a.getBlock()]._pages[a.getPage()].incrStat(stat);
}

void FlashSystem::blockStatIncr(Address a, blockStat_t stat)
{
#ifdef DEBUG
  if(!addressRangeCheck(a), BLOCK_LEVEL)
    em.error("Blockstatincr failed address range check");
#endif /* DEBUG */

  _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
    _blocks[a.getBlock()].incrStat(stat);
}

void FlashSystem::planeStatIncr(Address a, planeStat_t stat)
{
#ifdef DEBUG
  if(!addressRangeCheck(a), PLANE_LEVEL)
    em.error("PlaneStatIncr failed address range check");
#endif /* DEBUG */

  _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].incrStat(stat);
}

void FlashSystem::lunStatIncr(Address a, lunStat_t stat)
{
#ifdef DEBUG
  if(!addressRangeCheck(a), LUN_LEVEL)
    em.error("LunStatIncr failed address range check");
#endif /* DEBUG */

  _channels[a.getChannel()]._luns[a.getLun()].incrStat(stat);
}

void FlashSystem::channelStatIncr(Address a, channelStat_t stat)
{
#ifdef DEBUG
  if(!addressRangeCheck(a), CHANNEL_LEVEL)
    em.error("ChannelStatIncr failed address range check");
#endif /* DEBUG */

  _channels[a.getChannel()].incrStat(stat);
}

void FlashSystem::flashSystemStatIncr(flashSystemStat_t stat)
{
  _stats[stat]++;
}

int FlashSystem::getBlockLastWrittenOffset(Address a)
{
  int res;

  if(!addressRangeCheck(a, BLOCK_LEVEL))
  {
      string error = "Get block last written offset address range check"
           " failed @" + a.str();
      ERROR(error);
  }

  res = _channels[a.getChannel()]._luns[a.getLun()]._planes[a.getPlane()].
      _blocks[a.getBlock()]._lastPageWrittenOffset;

  return res;
}

FlashSystem * FlashSystem::getInstance ()
{
  if (_singleton == NULL)
      _singleton =  new FlashSystem(Param::getInstance()->getInt("flash_layer.channels"),
                                    Param::getInstance()->getInt("flash_layer.luns_per_channel"),
                                    Param::getInstance()->getInt("flash_layer.planes_per_lun"),
                                    Param::getInstance()->getInt("flash_layer.blocks_per_plane"),
                                    Param::getInstance()->getInt("flash_layer.pages_per_block"));

  return _singleton;
}

void FlashSystem::kill ()
{
  if (_singleton != NULL)
    {
      delete _singleton;
      _singleton = NULL;
    }
}

int FlashSystem::getIoCycles()
{
  return _ioCycles;
}
