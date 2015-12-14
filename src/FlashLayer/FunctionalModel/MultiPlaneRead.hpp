/*
 * MultiPlaneRead.h
 *
 *  Created on: 12 sept. 2013
 *      Author: pierre
 */

#ifndef MULTIPLANEREAD_HPP
#define MULTIPLANEREAD_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class MultiPlaneRead : public UniLunCmd
{
public:
  MultiPlaneRead(vector<Address> addresses, FlashSystem *f=FlashSystem::getInstance());
  MultiPlaneRead(Address address, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiPlaneRead();

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

  vector<double> _tons;
  vector<double> _ios;
  vector<double> _tps;

  double tp(int i);
};

#endif /* MULTIPLANEREAD_HPP */
