/*
 * CacheRead.hpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#ifndef CACHEWRITE_HPP
#define CACHEWRITE_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class CacheWrite : public UniLunCmd
{
public:
  CacheWrite(Address startPage, int nbPagesToWrite, FlashSystem *f=FlashSystem::getInstance());
  virtual ~CacheWrite();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  int getChannelIndex();
  int getLunIndex();
  vector<multiLunSubJob_t> getMultiLunSubJobs();

private:
  Address _startPage;
  int _nbPagesToWrite;

  vector<double> _ios;
  vector<double> _tins;

};

#endif /* CACHEWRITE_HPP */
