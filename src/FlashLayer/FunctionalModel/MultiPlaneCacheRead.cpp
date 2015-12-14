/*
 * MultiPlaneCacheRead.cpp
 *
 *  Created on: 23 sept. 2013
 *      Author: pierre
 */

#include "MultiPlaneCacheRead.hpp"

/** todo merge initialization of both constructors into 1 */
MultiPlaneCacheRead::MultiPlaneCacheRead(Address startPage, int nbPagesToRead, FlashSystem *f) :
  UniLunCmd(f)
{
  int nbPlanes = _f->getStructuralParameter(SP_PLANES_PER_LUN);
  _mMax = nbPagesToRead;
  for(int i=0; i<nbPlanes; i++)
  {
    _startPages.push_back(startPage);
    _startPages[i].setPlane(i);
    _nbPagesToRead.push_back(nbPagesToRead);
  }

  _tmpEio.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEio[i].resize(_mMax, -1);

  _tmpEton.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEton[i].resize(_mMax, -1);

  _tons.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tons[i].resize(_mMax, -1);

  _ios.resize(nbPlanes);
    for(int i=0; i<nbPlanes; i++)
      _ios[i].resize(_mMax, -1);

}

MultiPlaneCacheRead::MultiPlaneCacheRead(vector<Address> startPages,
    vector<int> nbPagesToRead, FlashSystem *f) : UniLunCmd(f), _startPages(startPages),
    _nbPagesToRead(nbPagesToRead)
{
  _mMax = 0;
  for(int i=0; i<(int)_nbPagesToRead.size(); i++)
    if(_nbPagesToRead[i] > _mMax)
      _mMax = _nbPagesToRead[i];

  int nbPlanes = _startPages.size();
  _tmpEio.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEio[i].resize(_mMax, -1);

  _tmpEton.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tmpEton[i].resize(_mMax, -1);

  _tons.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
    _tons[i].resize(_mMax, -1);

  _ios.resize(nbPlanes);
    for(int i=0; i<nbPlanes; i++)
      _ios[i].resize(_mMax, -1);
}

MultiPlaneCacheRead::~MultiPlaneCacheRead(){}

void MultiPlaneCacheRead::constraintsCheck()
{
  /* arrays must have the same size */
  if(_startPages.size() != _nbPagesToRead.size())
    ERROR("The 2 arrays describing a multi plane cache read operation must "
        "have the same size");

  /* at least 2 planes involved */
  if(_startPages.size() < 2)
    ERROR("The number of planes involved in multi plane cache read "
        "operation must be at least 2");

  /* at least 2 pages read in cache mode */
  for(int i=0; i<(int)_nbPagesToRead.size(); i++)
    if(_nbPagesToRead[i] < 2)
      ERROR("The number of pages read in multi plane cache mode "
          "must be >= 2");

  /* each start page must be in a different plane of the same LUN */
  for(int i=0; i<(int)_startPages.size(); i++)
    for(int j=0; j<(int)_startPages.size(); j++)
      if(i != j)
      {
        if(_startPages[i].samePlane(_startPages[j]))
          ERROR("Each member of a multi plane cache read operation must "
              "target a different plane, here we have @" + _startPages[i].str() +
              " and @" + _startPages[j].str());

        if(!_startPages[i].sameLun(_startPages[j]))
          ERROR("Each member of a multi plane cache read operation must "
              "target the same LUN, here we have @" + _startPages[i].str() +
              " and @" + _startPages[j].str());
      }
}

void MultiPlaneCacheRead::addressRangeCheck()
{
  for(int i=0; i<(int)_startPages.size(); i++)
    if(!_f->addressRangeCheck(_startPages[i]))
      ERROR("Start page of multi plane cache read operation @" +
          _startPages[i].str() + " failed address range check");

  /* block boundaries */
  int nbPagesPerBlock = _f->getStructuralParameter(SP_PAGES_PER_BLOCK);
  for(int i=0; i<(int)_startPages.size(); i++)
    if((_startPages[i].getPage()+_nbPagesToRead[i]) > nbPagesPerBlock)
      ERROR("One cache read member of multi plane cache read operation "
          "crosses block boundaries: read " + nts(_nbPagesToRead[i]) + " pages"
          " from @" + _startPages[i].str());
}

void MultiPlaneCacheRead::updateFlashState()
{
  /* nothing to update for read operations */
}

void MultiPlaneCacheRead::updateFlashStats()
{
  for(int i=0; i<(int)_startPages.size(); i++)
  {
    Address a = _startPages[i];
    for(int j=a.getPage(); j<_nbPagesToRead[i]; j++)
    {
      a.setPage(j);
      _f->pageStatIncr(a, MP_CACHE_READ);
    }
  }

  _f->lunStatIncr(_startPages[0], MPCACHEREAD_NUM);
}

void MultiPlaneCacheRead::computePerfAndPc()
{
  _timeTaken = 0;
  _energyConsumed = 0;

  for(int i=0; i<(int)_startPages.size(); i++)
  {
    double t = eIo(i, (_mMax-1));
    if(t > _timeTaken)
      _timeTaken = t;

    for(int j=0; j<_nbPagesToRead[i]; j++)
    {
#if 1 /* debug */
      assert(_ios[i][j] != -1 && _tons[i][j] != -1);
#endif
      _energyConsumed += _tons[i][j]*PTON;
      _energyConsumed += _ios[i][j]*PIO;
    }
  }

#if 0 /* debug */
  for(int i=0; i<(int)_startPages.size(); i++)
  {
    for(int j=0; j<_nbPagesToRead[i]; j++)
      cout << "Eio[" << i <<"," << j << "]: " << eIo(i, j) << " ; ";
    cout << endl;
  }

  for(int i=0; i<(int)_startPages.size(); i++)
  {
    for(int j=0; j<_nbPagesToRead[i]; j++)
      cout << "Eton[" << i <<"," << j << "]: " << eTon(i, j) << " ; ";
    cout << endl;
  }

  for(int i=0; i<(int)_startPages.size(); i++)
  {
    for(int j=0; j<_nbPagesToRead[i]; j++)
      cout << "_tons[" << i <<"," << j << "]: " << _tons[i][j] << " ; ";
    cout << endl;
  }

  for(int i=0; i<(int)_startPages.size(); i++)
  {
    for(int j=0; j<_nbPagesToRead[i]; j++)
      cout << "_ios[" << i <<"," << j << "]: " << _ios[i][j] << " ; ";
    cout << endl;
  }
#endif

}

int MultiPlaneCacheRead::getChannelIndex()
{
  return _startPages[0].getChannel();
}

double MultiPlaneCacheRead::eIo(int i, int j)
{
  if(_tmpEio[i][j] != -1)
    return _tmpEio[i][j];

  if(j >= _nbPagesToRead[i])
  {
    _ios[i][j] = _tmpEio[i][j] = 0;
    return 0;
  }

  _ios[i][j] = IO;

  if(i==0 && j==0)
  {
    double res = eTon(0,0) + _ios[i][j];
    _tmpEio[i][j] = res;
    return res;
  }

  if(i==0)
  {
    double res = _ios[i][j];
    int n = _startPages.size();
    res += max(eIo(i,j-1),max(eIo(n-1, j-1), eTon(0, j)));
    _tmpEio[i][j] = res;
    return res;
  }

  double res = _ios[i][j];
  res += max(eIo(i,j-1),max(eIo(i-1, j), eTon(i, j)));
  _tmpEio[i][j] = res;

  return res;
}

double MultiPlaneCacheRead::eTon(int i, int j)
{
  if(_tmpEton[i][j] != -1)
      return _tmpEton[i][j];

  if(j >= _nbPagesToRead[i])
  {
    _tons[i][j] = _tmpEton[i][j] = 0;
    return 0;
  }

  _tons[i][j] = TONOFF(_startPages[i].getPage()+j);

  if(j==0)
  {
    _tmpEton[i][j] = _tons[i][j];
    return _tons[i][j];
  }

  if(j==1)
  {
    double  res = _tons[i][j] + eTon(i, 0);
    _tmpEton[i][j] = res;
    return res;
  }

  double res = _tons[i][j] + max(eIo(i, j-2), eTon(i, j-1));
  _tmpEton[i][j] = res;
  return res;
}

int MultiPlaneCacheRead::getLunIndex()
{
  return _startPages[0].getLun();
}

vector<multiLunSubJob_t> MultiPlaneCacheRead::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;
  int n = _startPages.size();

#if 1 /* debug */
  for(int i=0; i<(int)_startPages.size(); i++)
    for(int j=0; j<_nbPagesToRead[i]; j++)
      assert(_ios[i][j] != -1 && _tons[i][j] != -1);
#endif

  multiLunSubJob_t s1 = {PAR, _tons[0][0]};
  res.push_back(s1);

  s1.type = SEQ;
  s1.time = IO;
  res.push_back(s1);

    for(int j=0; j<_mMax; j++)
      for(int i=0; i<(int)_startPages.size(); i++)
      {
        if(i==0 && j==0)
          continue;

        if(j>= _nbPagesToRead[i])
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
          s1.type = PAR;
          s1.time = diffCurPrev;
          res.push_back(s1);
        }

      s1.type = SEQ;
      s1.time = IO;
      res.push_back(s1);
    }

  return res;
}
