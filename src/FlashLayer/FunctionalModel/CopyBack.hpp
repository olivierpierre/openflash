/*
 * CopyBack.hpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#ifndef COPYBACK_HPP
#define COPYBACK_HPP

#include <vector>
#include "../../Globs/Common.hpp"
#include "UniLunCmd.hpp"

class CopyBack : public UniLunCmd
{
public:
  CopyBack(Address source, Address target, FlashSystem *f=FlashSystem::getInstance());
  virtual ~CopyBack();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  int getChannelIndex();
  int getLunIndex();
  vector<multiLunSubJob_t> getMultiLunSubJobs();

private:
  Address _source;
  Address _target;

  double _tin;
  double _ton;
};

#endif /* COPYBACK_HPP */
