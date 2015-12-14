/*
 * MultiPlaneCopyBack.hpp
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#ifndef MULTIPLANECOPYBACK_HPP
#define MULTIPLANECOPYBACK_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class MultiPlaneCopyBack : public UniLunCmd
{
public:
  MultiPlaneCopyBack(vector<Address> sources, vector<Address> target, FlashSystem *f=FlashSystem::getInstance());
  MultiPlaneCopyBack(Address source, Address target, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiPlaneCopyBack();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  int getChannelIndex();
  int getLunIndex();
  vector<multiLunSubJob_t> getMultiLunSubJobs();

private:
  vector<Address> _sources;
  vector<Address> _targets;

  vector<double> _tins;
  vector<double> _tons;
};

#endif /* MULTIPLANECOPYBACK_HPP */
