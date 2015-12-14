/*
 * CacheRead.cpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#include "CacheRead.hpp"

#include <vector>
#include <stdint.h>

using namespace std;

CacheRead::CacheRead(Address startPage, int nbPagesToRead, FlashSystem *f) :
  UniLunCmd(f), _startPage(startPage), _nbPagesToRead(nbPagesToRead)
{
  _ios.resize(nbPagesToRead);
  _tons.resize(nbPagesToRead);

  for(int i=0; i<nbPagesToRead; i++)
    _ios[i] = _tons[i] = -1;
}

CacheRead::~CacheRead(){}

void CacheRead::constraintsCheck()
{
  if(_nbPagesToRead < 2)
    ERROR("Number of pages to read in cache mode must be > 2");
}

void CacheRead::addressRangeCheck()
{
  bool res = _f->addressRangeCheck(_startPage, PAGE_LEVEL);
  if(!res)
  {
    string errmsg = "Start page for cache read op (@" + _startPage.str() +
        "failed address range check";
    ERROR(errmsg);
  }

  /* Must stay in block boundaries */
  if((_startPage.getPage() + _nbPagesToRead) > (int)_f->getStructuralParameter(SP_PAGES_PER_BLOCK))
    ERROR("Cache read command overlap block boundaries");
}

void CacheRead::updateFlashState()
{
  /* Nothing to update for a read operation */
}

void
CacheRead::updateFlashStats()
{
  int startOffset = _startPage.getPage();
  Address addr = _startPage;
  for(int i=0; i<_nbPagesToRead; i++)
  {
    addr.setPage(startOffset + i);
    _f->pageStatIncr(addr, CACHE_READ_NUM_PAGE);

  }

  _f->planeStatIncr(_startPage, CACHE_READ_NUM_PLANE);
}

void CacheRead::computePerfAndPc()
{
  _tons[0] = TONOFF(_startPage.getPage());

  _timeTaken = _tons[0];
  _energyConsumed = _tons[0]*PTON;

  for(int i=1; i<_nbPagesToRead; i++)
  {
    _tons[i] = TONOFF(_startPage.getPage()+i);
    _ios[i-1] = IO;
    _timeTaken += max(_tons[i], _ios[i-1]);
    _energyConsumed += _tons[i]*PTON + _ios[i-1]*PIO;
  }

  _ios[_nbPagesToRead-1] = IO;
  _timeTaken += _ios[_nbPagesToRead-1];
  _energyConsumed += _ios[_nbPagesToRead-1]*PIO;
}

int CacheRead::getChannelIndex()
{
  return _startPage.getChannel();
}

int CacheRead::getLunIndex()
{
  return _startPage.getLun();
}

vector<multiLunSubJob_t> CacheRead::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;

#if 1 /* debug */
  for(int i=0; i<_nbPagesToRead; i++)
    assert(_ios[i] != -1 && _tons[i] != -1);
#endif

  multiLunSubJob_t s1 = {PAR, _tons[0]};
  res.push_back(s1);

  for(int i=1; i<_nbPagesToRead; i++)
  {
    s1.type = SEQ; s1.time = _ios[i-1];
    res.push_back(s1);

    double potentialPar = max(0.0, _tons[i]-_ios[i-1]);
    if(potentialPar > 0)
    {
      s1.type = PAR;
      s1.time = potentialPar;
      res.push_back(s1);
    }
  }

  s1.type = SEQ;
  s1.time = _ios[_nbPagesToRead-1];
  res.push_back(s1);

  return res;
}
