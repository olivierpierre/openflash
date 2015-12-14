/*
 * MultiPlaneCacheWrite.cpp
 *
 *  Created on: 23 sept. 2013
 *      Author: pierre
 */

#include "MultiPlaneCacheWrite.hpp"

MultiPlaneCacheWrite::MultiPlaneCacheWrite(Address startPage, int nbPagesToWrite, FlashSystem *f) :
  UniLunCmd(f)
{
  int nbPlanes = _f->getStructuralParameter(SP_PLANES_PER_LUN);
  _mMax = nbPagesToWrite;
  for(int i=0; i<nbPlanes; i++)
  {
    _startPages.push_back(startPage);
    _startPages[i].setPlane(i);
    _nbPagesToWrite.push_back(nbPagesToWrite);
  }

  _tmpEtin.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEtin[i].resize(_mMax, -1);

  _tmpEio.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEio[i].resize(_mMax, -1);

  _tins.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tins[i].resize(_mMax, -1);

  _ios.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _ios[i].resize(_mMax, -1);

}

MultiPlaneCacheWrite::MultiPlaneCacheWrite(vector<Address> startPages,
    vector<int> nbPagesToWrite, FlashSystem *f) :
	UniLunCmd(f), _startPages(startPages),
    _nbPagesToWrite(nbPagesToWrite)
{
  _mMax = 0;
  for(int i=0; i<(int)_nbPagesToWrite.size(); i++)
    if(_nbPagesToWrite[i] > _mMax)
      _mMax = _nbPagesToWrite[i];

  int nbPlanes = _startPages.size();
  _tmpEtin.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEtin[i].resize(_mMax, -1);

  _tmpEio.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEio[i].resize(_mMax, -1);

  _tins.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tins[i].resize(_mMax, -1);

  _ios.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _ios[i].resize(_mMax, -1);
}

MultiPlaneCacheWrite::~MultiPlaneCacheWrite(){}

void MultiPlaneCacheWrite::constraintsCheck()
{
  /* arrays must have the same size */
  if(_startPages.size() != _nbPagesToWrite.size())
    ERROR("The 2 arrays describing a multi plane cache write operation must "
        "have the same size");

  /* at least 2 planes involved */
  if(_startPages.size() < 2)
    ERROR("The number of planes involved in multi plane cache write "
        "operation must be at least 2");

  /* at least 2 pages written in cache mode */
  for(int i=0; i<(int)_nbPagesToWrite.size(); i++)
    if(_nbPagesToWrite[i] < 2)
      ERROR("The number of pages written in (multi plane) cache mode "
          "must be >= 2");

  /* each start page must be in a different plane of the same LUN */
  for(int i=0; i<(int)_startPages.size(); i++)
    for(int j=0; j<(int)_startPages.size(); j++)
      if(i != j)
      {
        if(_startPages[i].samePlane(_startPages[j]))
          ERROR("Each member of a multi plane cache write operation must "
              "target a different plane, here we have @" + _startPages[i].str() +
              " and @" + _startPages[j].str());

        if(!_startPages[i].sameLun(_startPages[j]))
          ERROR("Each member of a multi plane cache write operation must "
              "target the same LUN, here we have @" + _startPages[i].str() +
              " and @" + _startPages[j].str());
      }
}

void MultiPlaneCacheWrite::addressRangeCheck()
{
  for(int i=0; i<(int)_startPages.size(); i++)
    if(!_f->addressRangeCheck(_startPages[i]))
      ERROR("Start page of multi plane cache write operation @" +
          _startPages[i].str() + " failed address range check");

  /* block boundaries */
  int nbPagesPerBlock = _f->getStructuralParameter(SP_PAGES_PER_BLOCK);
  for(int i=0; i<(int)_startPages.size(); i++)
    if((_startPages[i].getPage()+_nbPagesToWrite[i]) > nbPagesPerBlock)
      ERROR("One cache write member of multi plane cache write operation "
          "crosses block boundaries: read " + nts(_nbPagesToWrite[i]) + " pages"
          " from @" + _startPages[i].str());
}

void MultiPlaneCacheWrite::updateFlashState()
{
  for(int i=0; i<(int)_startPages.size(); i++)
  {
    Address a = _startPages[i];
    int base = a.getPage();
    for(int j=0; j<_nbPagesToWrite[j]; j++)
    {
      a.setPage(base + j);
      int res = _f->setWritten(a);

      if(res == -1)
        ERROR("Write on non free page during multi plane cache write");
      else if(res == -2)
        ERROR("Non sequential write in block during multi plane cache write");
    }
  }
}

void MultiPlaneCacheWrite::updateFlashStats()
{
  for(int i=0; i<(int)_startPages.size(); i++)
  {
    Address a = _startPages[i];
    for(int j=a.getPage(); j<_nbPagesToWrite[i]; j++)
    {
      a.setPage(j);
      _f->pageStatIncr(a, MP_CACHE_WRITE);
    }
  }

  _f->lunStatIncr(_startPages[0], MPCACHEWRITE_NUM);
}

void MultiPlaneCacheWrite::computePerfAndPc()
{
  _timeTaken = 0;
  _energyConsumed = 0;

  for(int i=0; i<(int)_startPages.size(); i++)
  {
    double t = eTin(i, _mMax-1);
    if(_timeTaken < t)
      _timeTaken = t;

    for(int j=0; j<(_nbPagesToWrite[i]); j++)
    {
      _energyConsumed += TINOFF(_startPages[i].getPage()+j)*PTIN;
      _energyConsumed += IO*PIO;
    }
  }

#if 0 /* debug */
  for(int i=0; i<(int)_startPages.size(); i++)
  {
    for(int j=0; j<_nbPagesToWrite[i]; j++)
      cout << "Eio[" << i <<"," << j << "]: " << eIo(i, j) << " ; ";
    cout << endl;
  }

  for(int i=0; i<(int)_startPages.size(); i++)
  {
    for(int j=0; j<_nbPagesToWrite[i]; j++)
      cout << "Etin[" << i <<"," << j << "]: " << eTin(i, j) << " ; ";
    cout << endl;
  }
#endif
}

int MultiPlaneCacheWrite::getChannelIndex()
{
  return _startPages[0].getChannel();
}

double MultiPlaneCacheWrite::eIo(int i, int j)
{
  if(_tmpEio[i][j] != -1)
    return _tmpEio[i][j];

  if(j >= _nbPagesToWrite[i])
  {
    _ios[i][j] = _tmpEio[i][j] = 0;
    return 0;
  }

  _ios[i][j] = IO;

  if(i==0 && j==0)
  {
    _tmpEio[i][j] = _ios[i][j];
    return _ios[i][j];
  }

  if(i==0)
  {
    int n = _startPages.size();
    if(j == 0 || j == 1)
    {
      double res = _ios[i][j] + eIo(n-1, j-1);
      _tmpEio[i][j] = res;
      return res;
    }
    else
    {
      double res = _ios[i][j] + max(eIo(n-1, j-1), eTin(0, j-2));
      _tmpEio[i][j] = res;
      return res;
    }
  }

  if(j == 0 || j == 1)
  {
    double res = _ios[i][j] + eIo(i-1, j);
    _tmpEio[i][j] = res;
    return res;
  }
  else
  {
    double res = _ios[i][j] + max(eIo(i-1, j), eTin(i, j-2));
    _tmpEio[i][j] = res;
    return res;
  }

  /* keep gcc happy */
  ERROR("shouldnt come here ...");
  return -1;
}

double MultiPlaneCacheWrite::eTin(int i, int j)
{
  if(_tmpEtin[i][j] != -1)
        return _tmpEtin[i][j];

  if(j >= _nbPagesToWrite[i])
  {
    _tins[i][j] = _tmpEtin[i][j] = 0;
    return 0;
  }

  _tins[i][j] = TINOFF(_startPages[i].getPage() + j);

  if(i==0 && j==0)
  {
    double res = eIo(0,0) + _tins[i][j];
    _tmpEtin[i][j] = res;
    return res;
  }

  double res = _tins[i][j] + max(eIo(i, j), eTin(i, j-1));
  _tmpEtin[i][j] = res;
  return res;
}

int MultiPlaneCacheWrite::getLunIndex()
{
  return _startPages[0].getLun();
}

vector<multiLunSubJob_t> MultiPlaneCacheWrite::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;
  int n = _startPages.size();

  multiLunSubJob_t s1 = {SEQ, _ios[0][0]};
  res.push_back(s1);

    for(int j=0; j<_mMax; j++)
      for(int i=0; i<(int)_startPages.size(); i++)
      {
        if(i==0 && j==0)
          continue;

        if(j>= _nbPagesToWrite[i])
          continue;

        int previousPlane = (i==0) ? (n-1) : (i-1);
        int previousPage = (i==0) ? (j-1) : j;

        double endPreviousIo = eIo(previousPlane, previousPage);

        while(endPreviousIo == 0)
        {
          previousPage = (previousPlane==0) ? (previousPage-1) : previousPage;
          previousPlane = (previousPlane==0) ? (n-1) : (previousPlane-1);
          endPreviousIo = eIo(previousPlane, previousPage);
        }

        double endCurrentIo = eIo(i,j);
        double diffCurPrev = (endCurrentIo-IO) - endPreviousIo;

        if(diffCurPrev > 0)
        {
#if 0 /* debug */
          cout << "before (" << i << "," << j << ")" << endl;
          cout << "end prev: " << endPreviousIo << endl;
          cout << "end cur: " << endCurrentIo << endl;
#endif
          s1.type = PAR;
          s1.time = diffCurPrev;
          res.push_back(s1);
        }

#if 1 /* debug */
      assert(_ios[i][j] != -1);
#endif

      s1.type = SEQ;
      s1.time = _ios[i][j];
      res.push_back(s1);
    }

    double eLastIo = 0;
    double eLastTin = 0;
    for(int i=0; i<(int)_startPages.size(); i++)
      for(int j=0; j<_nbPagesToWrite[i]; j++)
      {
        double endIo = eIo(i, j);
        if(endIo > eLastIo)
          eLastIo = endIo;

        double endTin = eTin(i, j);
          if(endTin > eLastTin)
            eLastTin = endTin;
      }

    s1.type = PAR;
    s1.time = eLastTin - eLastIo;
    res.push_back(s1);

  return res;
}
