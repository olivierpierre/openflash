/*
 * LegacyWrite.cpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#include "LegacyWrite.hpp"

#include <assert.h> /* debug */

LegacyWrite::LegacyWrite(Address a, FlashSystem *f) : UniLunCmd(f), _address(a), _tin(-1), _io(-1) {}
LegacyWrite::~LegacyWrite() {}

void LegacyWrite::constraintsCheck()
{
  /* no particular constraint here */
}

void LegacyWrite::addressRangeCheck()
{
  if(!_f->addressRangeCheck(_address, PAGE_LEVEL))
  {
    string error = "Address range check failed for legacy write @ "
        + _address.str();
    ERROR(error);
  }
}

void LegacyWrite::updateFlashState()
{
  int res = _f->setWritten(_address);

  if(res == -1)
    ERROR("write on non free page during legacy write");
  else if(res == -2)
    ERROR("non sequential write in a block during legacy write");
}

void LegacyWrite::updateFlashStats()
{
  _f->pageStatIncr(_address, LEGACY_WRITE_NUM);
}

void LegacyWrite::computePerfAndPc()
{
  /* default */
  _tin = TINOFF(_address.getPage());
  _io = IO;
  _timeTaken = _io + _tin;
  _energyConsumed = _io*PIO + _tin*PTIN;

#ifdef VERBOSE
  cout << "[Flash] LegacyWrite @" << _address << " : " << _timeTaken << " us" << endl;
#endif /* VERBOSE */
}

ostream& operator<<(ostream& os, LegacyWrite& lr)
{
  os << "Legacy Write @ " << lr._address;
  return os;
}

int LegacyWrite::getChannelIndex()
{
  return _address.getChannel();
}

int LegacyWrite::getLunIndex()
{
  return _address.getLun();
}

vector<multiLunSubJob_t> LegacyWrite::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res(2);

#if 1 /* debug */
  assert(_io != -1 && _tin != -1);
#endif

  multiLunSubJob_t s1 = {SEQ, _io};
  multiLunSubJob_t s2 = {PAR, _tin};

  res[0] = s1; res[1] = s2;

  return res;
}
