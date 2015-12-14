/*
 * MultiLunCmd.hpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef MULTILUNCMD_HPP
#define MULTILUNCMD_HPP

#include "UniChannelCmd.hpp"

class UniLunCmd;

class MultiLunCmd : public UniChannelCmd
{
public:
  MultiLunCmd(vector<UniLunCmd *> commands, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiLunCmd();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  bool isMultiLun();
  int getChannelIndex();

private:
   vector<UniLunCmd *> _commands;

   double computeMultiLunPerfs();
};

#endif /* MULTILUNCMD_HPP */
