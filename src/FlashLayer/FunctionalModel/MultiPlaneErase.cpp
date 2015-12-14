/*
 * MultiPlaneErase.cpp
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#include "MultiPlaneErase.hpp"

MultiPlaneErase::MultiPlaneErase(vector<Address> addresses, FlashSystem *f) :
  UniLunCmd(f), _addresses(addresses)
{
  _bers.resize(addresses.size());

  for(int i=0; i<(int)_bers.size(); i++)
    _bers[i] = 0;
}

MultiPlaneErase::MultiPlaneErase(Address address, FlashSystem *f) : UniLunCmd(f)
{
  int nbPlanes = _f->getStructuralParameter(SP_PLANES_PER_LUN);
  _bers.resize(nbPlanes);

  for(int i=0; i<nbPlanes; i++)
  {
    Address addr = address;
    addr.setPlane(i);
    _addresses.push_back(addr);
    _bers[i] = -1;
  }
}
MultiPlaneErase::~MultiPlaneErase(){}

void MultiPlaneErase::constraintsCheck()
{

  if(_addresses.size() < 2)
    ERROR("Number of planes involved in muti plane erase must be >= 2");

  for(int i=0; i<(int)_addresses.size(); i++)
  {
    for(int j=0; j<(int)_addresses.size(); j++)
      if(i!=j)
      {
        /* all addresses must be in different planes */
        if(_addresses[i].samePlane(_addresses[j]))
        {
          string errmsg = "Two members of multi plane erase are in the "
              "same plane: @" + _addresses[i].str() + " and @" + _addresses[j].str();
          ERROR(errmsg);
        }

        /* all addresses must be in the same lun*/
        if(!_addresses[i].sameLun(_addresses[j]))
        {
          string errmsg = "Two members of multi plane erase are not in the "
              "same lun: @" + _addresses[i].str() + " and @" + _addresses[j].str();
          ERROR(errmsg);
        }

        /* block index within the planes must be equal */
        if(Param::getInstance()->getBool("functional_model.multiplane_same_block"))
          if(_addresses[i].getBlock() != _addresses[j].getBlock())
          {
            string errmsg = "Two members of multi plane erase have not the "
                "same block offset within their plane: @" + _addresses[i].str()
                + " and @" + _addresses[j].str();
            ERROR(errmsg);
          }
      }
  }
}

void MultiPlaneErase::addressRangeCheck()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    if(!_f->addressRangeCheck(_addresses[i], BLOCK_LEVEL))
    {
      string errmsg = "Multi plane erase target @" + _addresses[i].str() +
          " failed address range check";
      ERROR(errmsg);
    }
}

void MultiPlaneErase::updateFlashState()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    _f->setErased(_addresses[i]);
}

void MultiPlaneErase::updateFlashStats()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    _f->blockStatIncr(_addresses[i], MULTIPLANE_ERASE_BLOCK);

  _f->lunStatIncr(_addresses[0], MULTIPLANE_ERASE_NUM);
}

void MultiPlaneErase::computePerfAndPc()
{
  int nbPlanes = _addresses.size();

  _timeTaken = 0;
  _energyConsumed = 0;
  for(int i=0; i<nbPlanes; i++)
  {
    _bers[i] = BERS;
    if(_bers[i] > _timeTaken)
      _timeTaken = _bers[i];
    _energyConsumed += _bers[i]*PBERS;
  }
}

int MultiPlaneErase::getChannelIndex()
{
  return _addresses[0].getChannel();
}

int MultiPlaneErase::getLunIndex()
{
  return _addresses[0].getLun();
}

vector<multiLunSubJob_t> MultiPlaneErase::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res(1);

#if 1 /* debug */
  for(int i=0; i<(int)_addresses.size(); i++)
    assert(_bers[i] != -1);
#endif

  double maxPar = 0;
  for(int i=0; i<(int)_addresses.size(); i++)
    if(_bers[i] > maxPar)
      maxPar = _bers[i];

  multiLunSubJob_t s1 = {PAR, maxPar};

  res[0] = s1;
  return res;
}
