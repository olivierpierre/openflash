/*
 * LegacyErase.hpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef LEGACYERASE_HPP
#define LEGACYERASE_HPP

#include "UniLunCmd.hpp"

#include <vector>

using namespace std;

class LegacyErase : public UniLunCmd
{
public:
  LegacyErase(Address a, FlashSystem *f=FlashSystem::getInstance());
  virtual ~LegacyErase();

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

  double _bers;

friend ostream& operator<<(ostream& os, LegacyErase& lr);
};

#endif /* LEGACYERASE_HPP */
