/*
 * FlashCmd.h
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef FLASHCMD_HPP
#define FLASHCMD_HPP

#include "../FlashSystem.hpp"
#include "../PpcGlobals.hpp"

#include <assert.h> /* debug */

class FlashCmd
{
public:
  FlashCmd(FlashSystem *f);
  virtual ~FlashCmd()=0;

  double getTimeTaken();
  double getEnergyConsumed();

  virtual bool isMultiChannel();
  virtual bool isMultiLun();

protected:
  FlashSystem *_f;
  double _timeTaken;
  double _energyConsumed;
  bool _isRan;

  void check();
  virtual void constraintsCheck()=0;
  virtual void addressRangeCheck()=0;

  void execute();
  virtual void updateFlashState()=0;
  virtual void updateFlashStats()=0;

  void checkAndExec();
  virtual void computePerfAndPc() = 0;

  /* utilities */
  void checkWriteFreeSeq(Address a);

  friend class MultiChannelCmd;
  friend class MultiLunCmd;
};

#endif /* FLASHCMD_HPP */
