/*
 * CacheRead.hpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#ifndef CACHEREAD_HPP
#define CACHEREAD_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class CacheRead : public UniLunCmd
{
public:
  CacheRead(Address startPage, int nbPagesToRead, FlashSystem *f=FlashSystem::getInstance());
  virtual ~CacheRead();

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
  int _nbPagesToRead;

  vector<double> _ios;
  vector<double> _tons;
};

#endif /* CACHEREAD_HPP_ */
