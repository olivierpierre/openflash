/*
 * UniLunCmd.h
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef UNILUNCMD_HPP
#define UNILUNCMD_HPP

#include "UniChannelCmd.hpp"
#include "MultiLunCmd.hpp"

class UniLunCmd : public UniChannelCmd
{
public:
  UniLunCmd(FlashSystem *f);
  virtual
  ~UniLunCmd();

  virtual int getLunIndex()=0;
  virtual vector<multiLunSubJob_t> getMultiLunSubJobs()=0;

};

#endif /* UNILUNCMD_HPP */
