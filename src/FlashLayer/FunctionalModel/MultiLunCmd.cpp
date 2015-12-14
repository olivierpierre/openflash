/*
 * MultiLunCmd.cpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#include "MultiLunCmd.hpp"
#include "UniLunCmd.hpp"

MultiLunCmd::MultiLunCmd(vector<UniLunCmd *> commands, FlashSystem *f) : UniChannelCmd(f),
  _commands(commands){}

MultiLunCmd::~MultiLunCmd(){}

bool MultiLunCmd::isMultiLun()
{
  return true;
}

void MultiLunCmd::constraintsCheck()
{
  for(int i=0; i<(int)_commands.size(); i++)
  {
    /* all commands must be of type uni lun */
    if(_commands[i]->isMultiChannel() || _commands[i]->isMultiLun())
          ERROR("A multi LUN command must not contain multi luns / multi "
              "channels commands");

      for(int j=0; j<(int)_commands.size(); j++)
        if(i!=j)
        {
          /* all commands must target the same channel */
          if(_commands[i]->getChannelIndex() != _commands[j]->getChannelIndex())
            ERROR("All commands in a multi LUN operation must target the "
                "same channel");
          /* all commands must target a different LUN */
          if(_commands[i]->getLunIndex() == _commands[j]->getLunIndex())
            ERROR("All commands in a multi LUN operation must target a "
                "different LUN");
        }

      _commands[i]->constraintsCheck();
  }
}

void MultiLunCmd::addressRangeCheck()
{
  for(int i=0; i<(int)_commands.size(); i++)
    _commands[i]->addressRangeCheck();
}

void MultiLunCmd::updateFlashState()
{
  for(int i=0; i<(int)_commands.size(); i++)
    _commands[i]->updateFlashState();
}

void MultiLunCmd::updateFlashStats()
{
  for(int i=0; i<(int)_commands.size(); i++)
    _commands[i]->updateFlashStats();

  Address a(0,0,0,0,0);
  a.setChannel(_commands[0]->getChannelIndex());
  _f->channelStatIncr(a, MULTI_LUN_NUM);
}

void MultiLunCmd::computePerfAndPc()
{
  for(int i=0; i<(int)_commands.size(); i++)
    _commands[i]->computePerfAndPc();

  /* time */
  _timeTaken = computeMultiLunPerfs();

  /* power consumption */
  _energyConsumed = 0;
  for(int i=0; i<(int)_commands.size(); i++)
    _energyConsumed += _commands[i]->getEnergyConsumed();
}

int MultiLunCmd::getChannelIndex()
{
  return _commands[0]->getChannelIndex();
}

double MultiLunCmd::computeMultiLunPerfs()
{
  int maxJobs = 0;  /* mnumber of jobs in the command with the maximum number of jobs*/
  vector<vector<multiLunSubJob_t> > jobs(_commands.size());

  /* get the seq / par series */
  for(int i=0; i<(int)_commands.size(); i++)
  {
    jobs[i] = _commands[i]->getMultiLunSubJobs();
    if((int)jobs[i].size() > maxJobs)
      maxJobs = jobs[i].size();
  }

  double globalCursor = 0;
  vector<double> localCursors(_commands.size(), 0);

  for(int j=0; j<maxJobs; j++)
    for(int i=0; i<(int)_commands.size(); i++)
    {
      if(j >= (int)jobs[i].size())
        continue;

      if(jobs[i][j].type == SEQ)
      {
        if(globalCursor < localCursors[i])
          globalCursor = localCursors[i];
        globalCursor += jobs[i][j].time;
      }

      localCursors[i] += jobs[i][j].time;
    }

  double res = 0;
  for(int i=0; i<(int)_commands.size(); i++)
    if(localCursors[i] > res)
      res = localCursors[i];


#if 0 /* debug */
  for(int i=0; i<(int)jobs.size(); i++)
  {
    for(int j=0; j<(int)jobs[i].size(); j++)
    {
      if(jobs[i][j].type==PAR)
        cout << "PAR " << jobs[i][j].time << " ; ";
      else
        cout << "SEQ " << jobs[i][j].time << " ; ";
    }
    cout << endl;
  }
#endif


  return res;
}
