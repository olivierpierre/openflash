/*
 * MultiPlaneCopyBack.cpp
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

//TODO : allow a number of operations < number of planes per block

#include "MultiPlaneCopyBack.hpp"

#include <vector>

using namespace std;

MultiPlaneCopyBack::MultiPlaneCopyBack(vector<Address> sources, vector<Address> targets, FlashSystem *f) :
  UniLunCmd(f), _sources(sources), _targets(targets)
{
  int nbPlanes = _sources.size();

  _tins.resize(nbPlanes); _tons.resize(nbPlanes);

  for(int i=0; i<nbPlanes; i++)
    _tins[i] = _tons[i] = -1;
}

MultiPlaneCopyBack::MultiPlaneCopyBack(Address source, Address target, FlashSystem *f) :
  UniLunCmd(f)
{
  vector<Address> sources; vector<Address> targets;
  Address source2 = source, target2 = target;

  int nbPlanes = _f->getStructuralParameter(SP_PLANES_PER_LUN);
  _tins.resize(nbPlanes); _tons.resize(nbPlanes);
  for(int i=0; i<nbPlanes; i++)
  {
    source2.setPlane(i);
    sources.push_back(source2);

    target2.setPlane(i);
    targets.push_back(target2);

    _tins[i] = _tons[i] = -1;
  }

  _sources = sources;
  _targets = targets;
}

MultiPlaneCopyBack::~MultiPlaneCopyBack(){}

void MultiPlaneCopyBack::constraintsCheck()
{
  /* obviously arrays must have the same size */
  if(_sources.size() != _targets.size())
    ERROR("Source and target arrays of multi plane copy "
        "back operation must have the same size");

  /* at least 2 planes involved */
  if(_sources.size() < 2)
    ERROR("Multi plane copy back must involve at least 2 planes");

  /* for each (src, dest) couple, src and dest must be in the same plane */
  for(int i=0; i<(int)_sources.size(); i++)
    if(!_sources[i].samePlane(_targets[i]))
      ERROR("Source (" + _sources[i].str() + ") and target " +
          _targets[i].str() + ") of multi plane copy back must target the "
              "same plane");

  /* each couple must target a different plane from the other couples */
  for(int i=0; i<(int)_sources.size(); i++)
    for(int j=0; j<(int)_sources.size(); j++)
      if(i!=j)
        if(_sources[i].getPlane() == _sources[j].getPlane())
          ERROR("Each couple (src, dest) in a multi plane copy back "
              "operation must target a different plane from the other couples,"
              "here we have src1=" + _sources[i].str() + " and src2=" +
              _sources[j].str());

  /* all srcs must have the same LUN and page member, and even block if this
   * constraint is set to ON
   */
  int srcLun = _sources[0].getLun();
  int srcBLock = _sources[0].getBlock();
  int srcPage = _sources[0].getPage();
  for(int i=1; i<(int)_sources.size(); i++)
  {
    if(_sources[i].getLun() != srcLun || _sources[i].getPage() != srcPage)
      ERROR("All sources in multi plane copy back operation must have "
          "the same page and LUN address members, here we have "
          + _sources[0].str() + " and " + _sources[i].str());
    if(Param::getInstance()->getBool("functional_model.multiplane_same_block") &&
        _sources[i].getBlock() != srcBLock)
      ERROR("All sources in multi plane copy back operation must have "
                "the same block address member, here we have "
                + _sources[0].str() + " and " + _sources[i].str());
  }

  /* same thing for targets */
  int tgtLun = _targets[0].getLun();
  int tgtBLock = _targets[0].getBlock();
  int tgtPage = _targets[0].getPage();
  for(int i=1; i<(int)_targets.size(); i++)
  {
    if(_targets[i].getLun() != tgtLun || _targets[i].getPage() != tgtPage)
      ERROR("All targets in multi plane copy back operation must have "
          "the same page and LUN address members, here we have "
          + _targets[0].str() + " and " + _targets[i].str());
    if(Param::getInstance()->getBool("functional_model.multiplane_same_block") &&
        _targets[i].getBlock() != tgtBLock)
      ERROR("All targets in multi plane copy back operation must have "
                "the same block address member, here we have "
                + _targets[0].str() + " and " + _targets[i].str());
  }

  /* odd / even constraint */
  if(Param::getInstance()->getBool("functional_model.copyback_even_odd_constraint"))
    for(int i=0; i<(int)_sources.size(); i++)
      if(_sources[i].getPage()%2 != _targets[i].getPage()%2)
        ERROR("Target and source page addresses for multi plane copy "
            "back operation must be both odd or both even, here we have " +
            _sources[i].str() + " and " + _targets[i].str());
}

void MultiPlaneCopyBack::addressRangeCheck()
{
  for(int i=0; i<(int)_sources.size(); i++)
  {
    if(!_f->addressRangeCheck(_sources[i]))
    {
      string errmsg = "Source page of multiplane copy back @" + _sources[i].str() +
          " failed address range check";
      ERROR(errmsg);
    }

    if(!_f->addressRangeCheck(_targets[i]))
    {
      string errmsg = "Target page of multiplane copy back @" + _targets[i].str() +
          " failed address range check";
      ERROR(errmsg);
    }
  }
}

void MultiPlaneCopyBack::updateFlashState()
{
  for(int i=0; i<(int)_targets.size(); i++)
  {
   int res = _f->setWritten(_targets[i]);
   if(res == -1)
     ERROR("Write on non free page during multi plane copy back");
   else if(res == -2)
     ERROR("Non sequential write during multi plane copy back");
  }
}

void MultiPlaneCopyBack::updateFlashStats()
{
  for(int i=0; i<(int)_sources.size(); i++)
    _f->pageStatIncr(_sources[i], MPCOPYBACK_SRC);

  for(int i=0; i<(int)_targets.size(); i++)
      _f->pageStatIncr(_targets[i], MPCOPYBACK_TRGT);

  _f->lunStatIncr(_sources[0], MPCOPYBACK_NUM);
}

void MultiPlaneCopyBack::computePerfAndPc()
{
  int nbPlanes = _sources.size();
  _timeTaken = 0;
  _energyConsumed = 0;

  for(int i=0; i<nbPlanes; i++)
  {
    _tons[i] = TONOFF(_sources[i].getPage());
    _tins[i] = TINOFF(_targets[i].getPage());
    double time = _tons[i] + _tins[i];
    if(time > _timeTaken)
      _timeTaken = time;
    _energyConsumed += _tons[i]*PTON + _tins[i]*PTIN;
  }
}

int MultiPlaneCopyBack::getChannelIndex()
{
  return _sources[0].getChannel();
}

int MultiPlaneCopyBack::getLunIndex()
{
  return _sources[0].getLun();
}

vector<multiLunSubJob_t> MultiPlaneCopyBack::getMultiLunSubJobs()
{
  vector<multiLunSubJob_t> res;
  multiLunSubJob_t s1;
  double time = 0;

#if 1 /* debug */
  for(int i=0; i<(int)_sources.size(); i++)
    assert(_tins[i] != -1 && _tons[i] != -1);
#endif

  for(int i=0; i<(int)_sources.size(); i++)
  {
    double tmpTime = _tons[i] + _tins[i];
    if(tmpTime > time)
      time = tmpTime;
  }

  s1.type = PAR;
  s1.time = time;
  res.push_back(s1);
  return res;
}
