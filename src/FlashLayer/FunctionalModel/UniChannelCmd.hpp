/*
 * UniChannelCmd.h
 *
 *  Created on: 11 sept. 2013
 *      Author: pierre
 */

#ifndef UNICHANNELCMD_HPP
#define UNICHANNELCMD_HPP

#include "FlashCmd.hpp"

class UniChannelCmd : public FlashCmd
{
public:
  UniChannelCmd(FlashSystem *f);
  virtual ~UniChannelCmd();

  virtual int getChannelIndex()=0;
};

#endif /* UNICHANNELCMD_H */
