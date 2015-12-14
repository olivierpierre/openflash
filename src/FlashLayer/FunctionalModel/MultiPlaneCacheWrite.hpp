/*
 * MultiPlaneCacheWrite.hpp
 *
 *  Created on: 23 sept. 2013
 *      Author: pierre
 */

#ifndef MULTIPLANECACHEWRITE_HPP
#define MULTIPLANECACHEWRITE_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class MultiPlaneCacheWrite : public UniLunCmd
{
public:
  MultiPlaneCacheWrite(Address startPage, int nbPagesToWrite, FlashSystem *f=FlashSystem::getInstance());
  MultiPlaneCacheWrite(vector<Address> startPages, vector<int> nbPagesToWrite, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiPlaneCacheWrite();

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
  vector<int> _nbPagesToWrite;

  int _mMax;
  vector<vector<double> > _tmpEtin;
  vector<vector<double> > _tmpEio;
  vector<vector<double> > _tins;
  vector<vector<double> > _ios;

  double eTin(int i, int j);
  double eIo(int i, int j);
};

#endif /* MULTIPLANECACHEWRITE_HPP */
