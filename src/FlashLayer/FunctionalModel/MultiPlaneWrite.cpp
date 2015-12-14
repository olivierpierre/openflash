/*
 * MultiPlaneWrite.cpp
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#include "MultiPlaneWrite.hpp"

#include <vector>

using namespace std;

MultiPlaneWrite::MultiPlaneWrite(vector<Address> addresses, FlashSystem *f) :
  UniLunCmd(f), _addresses(addresses){}
MultiPlaneWrite::MultiPlaneWrite(Address address, FlashSystem *f) : UniLunCmd(f)
{
  for(int i=0; i<(int)_f->getStructuralParameter(SP_PLANES_PER_LUN); i++)
  {
    Address addr = address;
    addr.setPlane(i);
    _addresses.push_back(addr);
  }
}
MultiPlaneWrite::~MultiPlaneWrite(){}

void MultiPlaneWrite::constraintsCheck()
{
  /* at least 2 planes involved */
  if(_addresses.size() < 2)
    ERROR("Number of planes involved in multi plane write must be >= 2");

  for(int i=0; i<(int)_addresses.size(); i++)
  {
    for(int j=0; j<(int)_addresses.size(); j++)
      if(i!=j)
      {
        /* all addresses must be in different planes */
        if(_addresses[i].samePlane(_addresses[j]))
        {
          string errmsg = "Two members of multi plane write are in the "
              "same plane: @" + _addresses[i].str() + " and @" + _addresses[j].str();
          ERROR(errmsg);
        }

        /* all addresses must be in the same lun*/
        if(!_addresses[i].sameLun(_addresses[j]))
        {
          string errmsg = "Two members of multi plane write are not in the "
              "same lun: @" + _addresses[i].str() + " and @" + _addresses[j].str();
          ERROR(errmsg);
        }

        /* block index within the planes must be equal */
        if(Param::getInstance()->getBool("functional_model.multiplane_same_block"))
          if(_addresses[i].getBlock() != _addresses[j].getBlock())
          {
            string errmsg = "Two members of multi plane write have not the "
                "same block offset within their plane: @" + _addresses[i].str()
                + " and @" + _addresses[j].str();
            ERROR(errmsg);
          }
      }
  }
}

void MultiPlaneWrite::addressRangeCheck()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    if(!_f->addressRangeCheck(_addresses[i], PAGE_LEVEL))
    {
      string errmsg = "Multi plane write target @" + _addresses[i].str() +
          " failed address range check";
      ERROR(errmsg);
    }
}

void MultiPlaneWrite::updateFlashState()
{
  for(int i=0; i<(int)_addresses.size(); i++)
  {
    int res = _f->setWritten(_addresses[i]);
    if(res == -1)
      ERROR("Write in non free page during multi plane write");
    else if(res == -2)
      ERROR("Non sequential write during multi plane write");
  }
}

void MultiPlaneWrite::updateFlashStats()
{
  for(int i=0; i<(int)_addresses.size(); i++)
    _f->pageStatIncr(_addresses[i], MULTIPLANE_WRITE_PAGE);

  _f->lunStatIncr(_addresses[0], MULTIPLANE_WRITE_NUM);
}

void MultiPlaneWrite::computePerfAndPc()
{
  int nbPlanes = _addresses.size();

  _timeTaken = 0;
  _energyConsumed = 0;
  double ioSum = 0;
  for(int i=0; i<nbPlanes; i++)
  {
    double tmpIo = IO;
    double tmpTin = TINOFF(_addresses[i].getPage());

    ioSum += tmpIo;

    if((ioSum + tmpTin) > _timeTaken)
      _timeTaken = (ioSum + tmpTin);

    _energyConsumed += (tmpIo*PIO) + (tmpTin*PTIN);
  }
}

int MultiPlaneWrite::getChannelIndex()
{
  return _addresses[0].getChannel();
}

int MultiPlaneWrite::getLunIndex()
{
  return _addresses[0].getLun();
}

vector<multiLunSubJob_t> MultiPlaneWrite::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;
  multiLunSubJob_t s1 = {SEQ, IO};
  int n=_addresses.size();

  for(int i=0; i<n; i++)
    res.push_back(s1);

  double lastPar = 0;
  for(int i=0; i<n; i++)
  {
    double potentialPar = TINOFF(_addresses[i].getPage()) - (n-1-i)*IO;
    if(potentialPar > lastPar)
      lastPar = potentialPar;
  }

  s1.type = PAR;
  s1.time = lastPar;
  res.push_back(s1);

  return res;
}
