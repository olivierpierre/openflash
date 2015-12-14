/*
 * Lun.hpp
 *
 *  Created on: 30 août 2013
 *      Author: pierre
 */

#ifndef LUN_HPP_
#define LUN_HPP_

#include "Plane.hpp"

class Lun
{
public:
  Lun(int nbPlanes, int nbBlocksPerPlane, int nbPagesPerBlock);
  virtual ~Lun();

  void resetStats();

private:
  vector<Plane> _planes;
  uint64_t _stats[NUMBER_OF_LUN_STATS];

  void incrStat(lunStat_t stat);

  friend class FlashSystem;
};

#endif /* LUN_HPP_ */
