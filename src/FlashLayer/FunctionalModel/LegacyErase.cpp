/*
 * LegacyErase.cpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#include "LegacyErase.hpp"

LegacyErase::LegacyErase(Address a, FlashSystem *f) : UniLunCmd(f), _address(a), _bers(-1){}

LegacyErase::~LegacyErase(){}

void LegacyErase::constraintsCheck()
{
  // No particular constraint for legacy erase
}

void LegacyErase::addressRangeCheck()
{
  if(!_f->addressRangeCheck(_address, BLOCK_LEVEL))
  {
    string errmsg = "Legacy erase failed address range check @ " + _address.str();
    ERROR(errmsg);
  }
}

void LegacyErase::updateFlashState()
{
  _f->setErased(_address);
}

void LegacyErase::updateFlashStats()
{
  _f->blockStatIncr(_address, LEGACY_ERASE_NUM);
}

void LegacyErase::computePerfAndPc()
{
  _bers = BERS;
  _timeTaken = _bers;
  _energyConsumed = _bers*PBERS;

#ifdef VERBOSE
  cout << "[Flash] LegacyErase @" << _address << " : " << _timeTaken << " us" << endl;
#endif /* VERBOSE */
}

int LegacyErase::getChannelIndex()
{
  return _address.getChannel();
}

int LegacyErase::getLunIndex()
{
  return _address.getLun();
}

vector<multiLunSubJob_t> LegacyErase::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res(1);

#if 1 /* debug */
  assert(_bers != -1);
#endif

  multiLunSubJob_t s1 = {PAR, _bers};

  res[0] = s1;
  return res;
}
