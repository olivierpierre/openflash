/*
 * GlobalTime.cpp
 *
 *  Created on: 4 sept. 2013
 *      Author: pierre
 */

#include "GlobalTime.hpp"

TimeMgr *TimeMgr::_singleton = NULL;
TimeMgr *gtime;

TimeMgr::TimeMgr()  : _time (0.0) {}
TimeMgr::~TimeMgr() {}

void TimeMgr::incrTime(double val)
{
  _time += val;
}

void TimeMgr::setTime(double val)
{
  _time = val;
}

double TimeMgr::getTime()
{
  return _time;
}

TimeMgr * TimeMgr::getInstance ()
{
  if (_singleton == NULL)
      _singleton =  new TimeMgr();

  return _singleton;
}

void TimeMgr::kill ()
{
  if (_singleton != NULL)
    {
      delete _singleton;
      _singleton = NULL;
    }
}




