/*
 * LegacyRead.hpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef LEGACYREAD_HPP
#define LEGACYREAD_HPP

#include <vector>

#include "UniLunCmd.hpp"

using namespace std;

class LegacyRead : public UniLunCmd
{
public:
  LegacyRead(Address a, FlashSystem *f=FlashSystem::getInstance());
  virtual ~LegacyRead();

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
  double _io;
  double _ton;

friend ostream& operator<<(ostream& os, LegacyRead& lr);
};

#endif /* LEGACYREAD_HPP */
