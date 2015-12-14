/*
 * MultiChannelCmd.cpp
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#include "MultiChannelCmd.hpp"

MultiChannelCmd::MultiChannelCmd(vector<UniChannelCmd *> commands, FlashSystem *f) :
  FlashCmd(f), _commands(commands){}
MultiChannelCmd::~MultiChannelCmd(){}

void MultiChannelCmd::constraintsCheck()
{
  /* must be composed of uni channel commands */
  for(int i=0; i<(int)_commands.size(); i++)
  {
    if(_commands[i]->isMultiChannel())
      ERROR("A multi channel command can not contain another multi "
          "channel command");

    _commands[i]->constraintsCheck();

    /* all commands must target different channels */
    for(int j=0; j<(int)_commands.size(); j++)
      if(i!=j)
        if(_commands[i]->getChannelIndex() == _commands[j]->getChannelIndex())
          ERROR("Two commands composing a multi channel operation target "
              "the same channel");
  }
}

void MultiChannelCmd::addressRangeCheck()
{
  for(int i=0; i<(int)_commands.size(); i++)
    _commands[i]->addressRangeCheck();
}

void MultiChannelCmd::updateFlashState()
{
  for(int i=0; i<(int)_commands.size(); i++)
    _commands[i]->updateFlashState();
}

void MultiChannelCmd::updateFlashStats()
{
  for(int i=0; i<(int)_commands.size(); i++)
  {
    _commands[i]->updateFlashStats();
    _commands[i]->_isRan = true;
  }

  _f->flashSystemStatIncr(MULTI_CHANNEL_NUM);
}

void MultiChannelCmd::computePerfAndPc()
{
  _timeTaken = 0; _energyConsumed = 0;
  for(int i=0; i<(int)_commands.size(); i++)
  {
    _commands[i]->computePerfAndPc();

    if(_commands[i]->getTimeTaken() > _timeTaken)
      _timeTaken = _commands[i]->getTimeTaken();

    _energyConsumed += _commands[i]->getEnergyConsumed();
  }
}

bool MultiChannelCmd::isMultiChannel()
{
  return true;
}
