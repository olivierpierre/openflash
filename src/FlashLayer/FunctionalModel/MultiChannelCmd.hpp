/*
 * MultiChannelCmd.h
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#ifndef MULTICHANNELCMD_HPP
#define MULTICHANNELCMD_HPP

#include "FlashCmd.hpp"
#include "UniChannelCmd.hpp"

class MultiChannelCmd : public FlashCmd
{
public:
  MultiChannelCmd(vector<UniChannelCmd *> commands, FlashSystem *f=FlashSystem::getInstance());
  virtual ~MultiChannelCmd();

  void constraintsCheck();
  void addressRangeCheck();
  void updateFlashState();
  void updateFlashStats();

  void computePerfAndPc();

  bool isMultiChannel();

private:
  vector<UniChannelCmd *> _commands;
};

#endif /* MULTICHANNELCMD_HPP */
