/*
 * MultiPlaneRead.cpp
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#include <vector>

#include "MultiPlaneRead.hpp"

using namespace std;

MultiPlaneRead::MultiPlaneRead(vector<Address> addresses, FlashSystem *f) :
  UniLunCmd(f), _addresses(addresses)
{
  int nbPlanes = _addresses.size();
  _ios.resize(nbPlanes);
  _tons.resize(nbPlanes);
  _tps.resize(nbPlanes);

  for(int i=0; i<nbPlanes; i++)
    _ios[i] = _tons[i] = _tps[i] = -1;
}

MultiPlaneRead::MultiPlaneRead(Address address, FlashSystem *f) : UniLunCmd(f)
{
  int nbPlanes = _f->getStructuralParameter(SP_PLANES_PER_LUN);

  _ios.resize(nbPlanes);
  _tons.resize(nbPlanes);
  _tps.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
  {
    Address addr = address;
    addr.setPlane(i);
    _addresses.push_back(addr);

    _ios[i] = _tons[i] = _tps[i] = -1;
  }


}
MultiPlaneRead::~MultiPlaneRead(){}

void MultiPlaneRead::constraintsCheck()
{
  /* at least 2 planes involved */
  if(_addresses.size() < 2)
    ERROR("Number of planes involved in multi plane read must be >= 2");

  /* all addresses must be in different planes of the same LUN */
  for(int i=0; i<(int)_addresses.size(); i++)
    for(int j=0; j<(int)_addresses.size(); j++)
      if(i!=j)
      {
        if(_addresses[i].samePlane(_addresses[j]))
        {
          string errmsg = "Two members of multi plane read are in the "
              "same plane: @" + _addresses[i].str() + " and @" + _addresses[j].str();
          ERROR(errmsg);
        }

        if(!_addresses[i].sameLun(_addresses[j]))
        {
          string errmsg = "Two members of multi plane read are not in the "
              "same lun: @" + _addresses[i].str() + " and @" + _addresses[j].str();
          ERROR(errmsg);
        }

        if(Param::getInstance()->getBool("functional_model.multiplane_same_block"))
          if(_addresses[i].getBlock() != _addresses[j].getBlock())
          {
            string errmsg = "Two members of multi plane read have not the "
                "same block offset within their plane: @" + _addresses[i].str()
                + " and @" + _addresses[j].str();
            ERROR(errmsg);
          }
      }
}

void MultiPlaneRead::addressRangeCheck()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    if(!_f->addressRangeCheck(_addresses[i], PAGE_LEVEL))
    {
      string errmsg = "Multi plane read target @" + _addresses[i].str() +
          " failed address range check";
      ERROR(errmsg);
    }
}

void MultiPlaneRead::updateFlashState()
{
  /* nothing to update for read operations */
}

void
MultiPlaneRead::updateFlashStats()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    _f->pageStatIncr(_addresses[i], MULTIPLANE_READ_PAGE);

  _f->lunStatIncr(_addresses[0], MULTIPLANE_READ_NUM);
}

void MultiPlaneRead::computePerfAndPc()
{
  int nbPlanes = _addresses.size();

  vector<double> tpis(_addresses.size());
  for(int i=0; i<nbPlanes; i++)
    tpis[i] = tp(i);

  _timeTaken = 0;
  for(int i=0; i<nbPlanes; i++)
    if(tpis[i] > _timeTaken)
      _timeTaken = tpis[i];

  _energyConsumed = 0;
  for(int i=0; i<nbPlanes; i++)
    _energyConsumed += TONOFF(_addresses[i].getPage())*PTON;
  _energyConsumed += nbPlanes*IO*PIO;
}

/* recursive function see theoretical model */
double MultiPlaneRead::tp(int i)
{
  if(_tps[i] != -1)
    return _tps[i];

  double res;

  _tons[i] = TONOFF(_addresses[i].getPage());
  _ios[i] = IO;

  if(i==0)
    res = (_tons[0] + _ios[0]);
  else
    res = (max(tp(i-1), _tons[i])) + +_ios[i];

  _tps[i] = res;
  return res;
}

int MultiPlaneRead::getChannelIndex()
{
  return _addresses[0].getChannel();
}

int MultiPlaneRead::getLunIndex()
{
  return _addresses[0].getLun();
}

vector<multiLunSubJob_t> MultiPlaneRead::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;

#if 1 /* debug */
  for(int i=0; i<(int)_addresses.size(); i++)
    assert(_tons[i] != -1 && _ios[i] != -1);
#endif

  multiLunSubJob_t s1 = {PAR, _tons[0]};

  res.push_back(s1);

  s1.type = SEQ; s1.time = _ios[0];
  res.push_back(s1);

  for(int i=1; i<(int)_addresses.size(); i++)
  {
    double tiPrev = tp(i-1);
    double potentialPar = max(0.0, _tons[i]-tiPrev);
    if(potentialPar > 0)
    {
      s1.type = PAR;
      s1.time = potentialPar;
      res.push_back(s1);
    }

    s1.type = SEQ;
    s1.time = _ios[i];
    res.push_back(s1);
  }

  return res;
}
