/*
 * LegacyRead.cpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#include "LegacyRead.hpp"
#include "UniLunCmd.hpp"


#include <assert.h> /* debug */

LegacyRead::LegacyRead(Address a, FlashSystem *f) : UniLunCmd(FlashSystem::getInstance()), _address(a), _io(-1), _ton(-1){}
LegacyRead::~LegacyRead(){}

void LegacyRead::constraintsCheck()
{
  // No particular constraint for legacy read
}

void LegacyRead::addressRangeCheck()
{
  if(!_f->addressRangeCheck(_address, PAGE_LEVEL))
  {
    string error = "Address range check failed for legacy read @ "
        + _address.str();
    ERROR(error);
  }
}

void LegacyRead::updateFlashState()
{
  // Nothing to update for a read operation
}

void LegacyRead::updateFlashStats()
{
  _f->pageStatIncr(_address, LEGACY_READ_NUM);
}

void LegacyRead::computePerfAndPc()
{
  /* default */
  _ton = TONOFF(_address.getPage());
  _io = IO;
  _timeTaken = _ton + _io;
  _energyConsumed = (_ton/1000000.0)*PTON + (_io/1000000.0)*PIO;
#ifdef VERBOSE
  cout << "[Flash] LegacyRead @" << _address << " : " << _timeTaken << " us" << endl;
#endif /* VERBOSE */
}

ostream& operator<<(ostream& os, LegacyRead& lr)
{
  os << "Legacy Read @ " << lr._address;
  return os;
}

int LegacyRead::getChannelIndex()
{
  return _address.getChannel();
}

int LegacyRead::getLunIndex()
{
  return _address.getLun();
}

vector<multiLunSubJob_t> LegacyRead::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res(2);

#if 1 /* debug */
  assert(_io != -1 && _ton != -1);
#endif

  multiLunSubJob_t s1 = {PAR, _ton};
  multiLunSubJob_t s2 = {SEQ, _io};

  res[0] = s1; res[1] = s2;

  return res;
}
