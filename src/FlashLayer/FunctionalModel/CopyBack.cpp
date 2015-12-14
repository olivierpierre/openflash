/*
 * CopyBack.cpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#include "CopyBack.hpp"

#include <vector>

using namespace std;

CopyBack::CopyBack(Address source, Address target, FlashSystem *f) : UniLunCmd(f),
  _source(source), _target(target), _tin(-1), _ton(-1){}

CopyBack::~CopyBack(){}

void CopyBack::constraintsCheck()
{
  /* source and target must be in the same plane */
  if(_source.getPlane() != _target.getPlane() ||
      _source.getLun() != _target.getLun() ||
      _source.getChannel() != _target.getChannel())
  {
    string errmsg = "Source " + _source.str() + " and target " + _target.str()
        + " of copyback operation are not in the same plane";
    ERROR(errmsg);
  }

  /* even / odd address constraint */
  if(Param::getInstance()->getBool("functional_model.copyback_even_odd_constraint"))
    if(_source.getPage()%2 != _target.getPage()%2)
    {
      string errmsg = "Copy back source " + _source.str() + " and target " +
          _target.str() + " page offset in block must be both odd or both even";
      ERROR(errmsg);
    }

}

void CopyBack::addressRangeCheck()
{
  if(!_f->addressRangeCheck(_source, PAGE_LEVEL))
  {
    string errmsg = "Address range check failed for source page of copy back @"
        + _source.str();
    ERROR(errmsg);
  }

  if(!_f->addressRangeCheck(_target, PAGE_LEVEL))
    {
      string errmsg = "Address range check failed for target page of copy back @"
          + _target.str();
      ERROR(errmsg);
    }
}

void CopyBack::updateFlashState()
{
  int res = FlashSystem::getInstance()->setWritten(_target);

  if(res == -1)
  {
    ERROR("Write on non free page during copy back operation");
  }
  else if(res == -2)
  {
    ERROR("Non sequential write in a block during copy back operation");
  }
}

void CopyBack::updateFlashStats()
{
  _f->pageStatIncr(_source, COPYBACK_SRC);
  _f->pageStatIncr(_target, COPYBACK_TARGET);
  _f->planeStatIncr(_target, COPYBACK_NUM);
}

void CopyBack::computePerfAndPc()
{
  _ton = TONOFF(_source.getPage());
  _tin = TINOFF(_target.getPage());
  _timeTaken = _ton + _tin;
  _energyConsumed = _ton*PTON + _tin*PTIN;
}

int CopyBack::getChannelIndex()
{
  return _source.getChannel();
}

int CopyBack::getLunIndex()
{
  return _source.getLun();
}

vector<multiLunSubJob_t> CopyBack::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res(1);

#if 1 /* debug */
  assert(_tin != -1 && _ton != -1);
#endif

  multiLunSubJob_t s1 = {PAR, _ton  + _tin};

  res[0] = s1;
  return res;
}
