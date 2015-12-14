/*
 * CacheRead.cpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#include "CacheWrite.hpp"

#include <vector>

using namespace std;

CacheWrite::CacheWrite(Address startPage, int nbPagesToWrite, FlashSystem *f) :
  UniLunCmd(f), _startPage(startPage), _nbPagesToWrite(nbPagesToWrite)
{
  _ios.resize(nbPagesToWrite);
  _tins.resize(nbPagesToWrite);

  for(int i=0; i<_nbPagesToWrite; i++)
    _ios[i] = _tins[i] = -1;
}

CacheWrite::~CacheWrite(){}

void CacheWrite::constraintsCheck()
{
  /* at least 2 pages written in cache mode */
  if(_nbPagesToWrite < 2)
    ERROR("Number of pages to write in cache mode must be > 2");
}

void CacheWrite::addressRangeCheck()
{
  bool res = _f->addressRangeCheck(_startPage, PAGE_LEVEL);
  if(!res)
  {
    string errmsg = "Start page for cache write op (@" + _startPage.str() +
        "failed address range check";
    ERROR(errmsg);
  }

  /* Must stay in block boundaries */
  if((_startPage.getPage() + _nbPagesToWrite) > (int)_f->getStructuralParameter(SP_PAGES_PER_BLOCK))
    ERROR("Cache write command overlap block boundaries");
}

void CacheWrite::updateFlashState()
{
  Address addr = _startPage;

  int firstOffset = _startPage.getPage();
  for(int i=firstOffset; i<(_nbPagesToWrite+firstOffset); i++)
  {
    addr.setPage(i);
    int res = _f->setWritten(addr);

    if(res == -1)
      ERROR("Page " + addr.str() + " written in cache write is not free");
    else if(res == -2)
      ERROR("Page " + addr.str() + " written in cache write is not "
          "sequentially written");
  }
}

void
CacheWrite::updateFlashStats()
{
  Address addr = _startPage;

  int firstOffset = _startPage.getPage();
  for(int i=firstOffset; i<(_nbPagesToWrite+firstOffset); i++)
  {
    addr.setPage(i);
    _f->pageStatIncr(addr, CACHE_WRITE_NUM_PAGE);
  }

  _f->planeStatIncr(_startPage, CACHE_WRITE_NUM_PLANE);
}

void CacheWrite::computePerfAndPc()
{
  /* default */
  _ios[0] = IO;

  _timeTaken = _ios[0];
  _energyConsumed = _ios[0]*PIO;

  for(int i=0; i<(_nbPagesToWrite-1); i++)
  {
    _ios[i+1] = IO;
    _tins[i] = TINOFF(_startPage.getPage()+i);
    _timeTaken += max(_ios[i+1], _tins[i]);
    _energyConsumed  += _ios[i+1]*PIO + _tins[i]*PTIN;
  }

  _tins[_nbPagesToWrite-1] = TINOFF(_startPage.getPage()+_nbPagesToWrite-1);
  _timeTaken += _tins[_nbPagesToWrite-1];
  _energyConsumed  += _tins[_nbPagesToWrite-1]*PTIN;
}

int CacheWrite::getChannelIndex()
{
  return _startPage.getChannel();
}

int CacheWrite::getLunIndex()
{
  return _startPage.getLun();
}

vector<multiLunSubJob_t> CacheWrite::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;

#if 1 /* debug */
  for(int i=0; i<_nbPagesToWrite; i++)
    assert(_ios[i] != -1 && _tins[i] != -1);
#endif

  multiLunSubJob_t s1 = {SEQ, _ios[0]};
  res.push_back(s1);

  for(int i=0; i<(_nbPagesToWrite-1); i++)
  {
    s1.type = SEQ; s1.time = _ios[i+1];
    res.push_back(s1);

    double potentialPar = max(0.0, _tins[i]-_ios[i+1]);
    if(potentialPar > 0)
    {
      s1.type = PAR;
      s1.time = potentialPar;
      res.push_back(s1);
    }
  }

  s1.type = PAR;
  s1.time = _tins[_nbPagesToWrite-1];
  res.push_back(s1);

  return res;
}
