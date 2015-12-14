/*
 * MultiPlaneCacheRead.hpp
 *
 *  Created on: 23 sept. 2013
 *      Author: pierre
 */

#ifndef MULTIPLANECACHEREAD_HPP_
#define MULTIPLANECACHEREAD_HPP_

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class MultiPlaneCacheRead : public UniLunCmd
{
public:
  MultiPlaneCacheRead(Address startPage, int nbPagesToRead, FlashSystem *f=FlashSystem::getInstance());
  MultiPlaneCacheRead(vector<Address> startPages, vector<int> nbPagesToRead, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiPlaneCacheRead();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  int getChannelIndex();
  int getLunIndex();
  vector<multiLunSubJob_t> getMultiLunSubJobs();

private:
  vector<Address> _startPages;
  vector<int> _nbPagesToRead;

  int _mMax;
  vector<vector<double> > _tmpEio;
  vector<vector<double> > _tmpEton;
  vector<vector<double> > _tons;
  vector<vector<double> > _ios;
  double eIo(int i, int j);
  double eTon(int i, int j);
};

#endif /* MULTIPLANECACHEREAD_HPP_ */
