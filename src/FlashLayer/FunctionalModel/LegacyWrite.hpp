/*
 * LegacyWrite.h
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef LEGACYWRITE_HPP
#define LEGACYWRITE_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

typedef double(*legacyWritePerfFPTR)(int);
typedef double(*legacyWritePcFPTR)(int, double);

class LegacyWrite : public UniLunCmd
{
public:
  LegacyWrite(Address a, FlashSystem *f=FlashSystem::getInstance());
  virtual ~LegacyWrite();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();
  void computePerfAndPc();

  int getChannelIndex();
  int getLunIndex();
  vector<multiLunSubJob_t> getMultiLunSubJobs();

private:
  Address _address;

  double _tin;
  double _io;

friend ostream& operator<<(ostream& os, LegacyWrite& lr);
};

#endif /* LEGACYWRITE_HPP */
