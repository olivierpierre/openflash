/*
 * Lun.cpp
 *
 *  Created on: 30 août 2013
 *      Author: pierre
 */

#include "Lun.hpp"

Lun::~Lun(){}

Lun::Lun(int nbPlanes, int nbBlocksPerPlane, int nbPagesPerBlock)
{
  for(int i=0; i<nbPlanes; i++)
    _planes.push_back(Plane(nbBlocksPerPlane, nbPagesPerBlock));

  resetStats();
}

void Lun::resetStats()
{
  for(int i=0; i<NUMBER_OF_LUN_STATS; i++)
    _stats[i] = 0;

  for(int i=0; i<(int)_planes.size(); i++)
    _planes[i].resetStats();
}

void Lun::incrStat(lunStat_t stat)
{
  _stats[stat]++;
}
