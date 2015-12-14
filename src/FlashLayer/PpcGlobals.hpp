/*
 * PpcGlobals.hpp
 *
 *  Created on: 13 sept. 2013
 *      Author: pierre
 */

#ifndef PPCGLOBALS_HPP
#define PPCGLOBALS_HPP

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

typedef enum
{
  PPC_CONST,     /* constants */
  PPC_MINMAX,    /* Random value between 2 bounds */
  PPC_NORMAL,   /* normal distribution */
  PPC_EXP,      /* exponential distribution */
  PPC_ADDR      /* based on address */
}ppcMode_t;

typedef struct
{
  ppcMode_t mode;
  double val_1;
  double val_2;
}ppcVal_t;

class PpcGlobals
{
public:
  static PpcGlobals * getInstance();
  static void kill();

  double ton(int offset=0);
  double tin(int offset=0);
  double io();
  double bers();

  double pton();
  double ptin();
  double pio();
  double pbers();

private:
  PpcGlobals();
  ~PpcGlobals();

  double bimodalAddr(double min, double max, int offset);

  static PpcGlobals *_singleton;

  /* const vals */
  ppcVal_t _ton;
  ppcVal_t _tin;
  ppcVal_t _io;
  ppcVal_t _bers;
  ppcVal_t _pton;
  ppcVal_t _ptin;
  ppcVal_t _pio;
  ppcVal_t _pbers;

  int _randomSeed;
  gsl_rng * _randomGen;
};

#define TON       (PpcGlobals::getInstance()->ton())
#define TIN       (PpcGlobals::getInstance()->tin())
#define TONOFF(x) (PpcGlobals::getInstance()->ton(x))
#define TINOFF(x) (PpcGlobals::getInstance()->tin(x))
#define BERS      (PpcGlobals::getInstance()->bers())
#define IO        (PpcGlobals::getInstance()->io()*FlashSystem::getInstance()->getIoCycles())
#define PTON      (PpcGlobals::getInstance()->pton())
#define PTIN      (PpcGlobals::getInstance()->ptin())
#define PBERS     (PpcGlobals::getInstance()->pbers())
#define PIO       (PpcGlobals::getInstance()->pio())

#endif /* PPCGLOBALS_HPP */
