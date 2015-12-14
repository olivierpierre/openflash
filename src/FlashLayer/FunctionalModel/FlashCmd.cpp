/*
 * FlashCmd.cpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#include "FlashCmd.hpp"
#include "../FlashSystem.hpp"

FlashCmd::FlashCmd(FlashSystem *f)
{
  _f = f;
  _timeTaken = -1;
  _energyConsumed = -1;
  _isRan = false;
}

FlashCmd::~FlashCmd(){}

void FlashCmd::check()
{
  addressRangeCheck();
  constraintsCheck();
}

void FlashCmd::execute()
{
  updateFlashState();
  updateFlashStats();
}

void FlashCmd::checkAndExec()
{
  if(_isRan)
    return;

  check();
  execute();
  computePerfAndPc();

  _isRan = true;

  if(_timeTaken == -1 || _energyConsumed == -1)
    ERROR("Flash command implementation did not update time and / "
        "or energy value upon launch");
}

void FlashCmd::checkWriteFreeSeq(Address a)
{
  if(_f->getPageState(a) != PAGE_FREE)
  {
    string errmsg = "Write on non-free page @" + a.str();
    ERROR(errmsg);
  }

  int lastWrittenOffset = _f->getBlockLastWrittenOffset(a);
  if(a.getPage() != lastWrittenOffset+1)
  {
    string errmsg = "Non sequential write within block @" + a.str() +
        ", last written offset was " + nts(lastWrittenOffset);
    ERROR(errmsg);
  }
}

bool FlashCmd::isMultiChannel()
{
  return false;
}

bool FlashCmd::isMultiLun()
{
  return false;
}

double FlashCmd::getTimeTaken()
{
  if(!_isRan)
    checkAndExec();

  return _timeTaken;
}

double FlashCmd::getEnergyConsumed()
{
  if(!_isRan)
    checkAndExec();

  return _energyConsumed;
}
