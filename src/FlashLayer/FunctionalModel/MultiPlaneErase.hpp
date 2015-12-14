/*
 * MultiPlaneRead.h
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#ifndef MULTIPLANEERASE_HPP
#define MULTIPLANEERASE_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class MultiPlaneErase : public UniLunCmd
{
public:
  MultiPlaneErase(vector<Address> addresses, FlashSystem *f=FlashSystem::getInstance());
  MultiPlaneErase(Address address, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiPlaneErase();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  int getChannelIndex();
  int getLunIndex();
  vector<multiLunSubJob_t> getMultiLunSubJobs();

private:
  vector<Address> _addresses;

  vector<double> _bers;
};

#endif /* MULTIPLANEERASE_HPP */
