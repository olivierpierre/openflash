/*
 * GlobalTime.hpp
 *
 *  Created on: 4 sept. 2013
 *      Author: pierre
 *
 * Singleton
 */

#ifndef GLOBALTIME_HPP
#define GLOBALTIME_HPP

#include "../Globs/Common.hpp"

class TimeMgr
{
public:
  static TimeMgr * getInstance();
  static void kill();

  void incrTime(double val);
  void setTime(double val);
  double getTime();

private:
  TimeMgr();
  ~TimeMgr();

  double _time;
  static TimeMgr *_singleton;
};

extern TimeMgr *gtime;

#endif /* GLOBALTIME_HPP */
