/*
 * MultiPlaneRead.h
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#ifndef MULTIPLANEWRITE_HPP
#define MULTIPLANEWRITE_HPP

#include <vector>
#include "UniLunCmd.hpp"

using namespace std;

class MultiPlaneWrite : public UniLunCmd
{
public:
  MultiPlaneWrite(vector<Address> addresses, FlashSystem *f=FlashSystem::getInstance());
  MultiPlaneWrite(Address address, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiPlaneWrite();

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
};

#endif /* MULTIPLANEWRITE_HPP */
