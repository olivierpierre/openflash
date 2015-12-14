/*
 * PpcGlobals.cpp
 *
 *  Created on: 13 sept. 2013
 *      Author: pierre
 */

#include "PpcGlobals.hpp"
#include "../Globs/Common.hpp"
#include "../Globs/RanDist.hpp"

PpcGlobals *PpcGlobals::_singleton= NULL;
PpcGlobals *model;

PpcGlobals::PpcGlobals()
{
  vector<ppcVal_t *> vals;
  vals.push_back(&_ton);
  vals.push_back(&_tin);
  vals.push_back(&_io);
  vals.push_back(&_bers);
  vals.push_back(&_pton);
  vals.push_back(&_ptin);
  vals.push_back(&_pio);
  vals.push_back(&_pbers);

  vector<string> names;
  names.push_back("ppc_models.performance_model.TON");
  names.push_back("ppc_models.performance_model.TIN");
  names.push_back("ppc_models.performance_model.IO");
  names.push_back("ppc_models.performance_model.BERS");
  names.push_back("ppc_models.power_consumption_model.PTON");
  names.push_back("ppc_models.power_consumption_model.PTIN");
  names.push_back("ppc_models.power_consumption_model.PIO");
  names.push_back("ppc_models.power_consumption_model.PBERS");

  if(Param::getInstance()->getString("ppc_models.performance_model.IO.mode").compare("const"))
    ERROR("ppc_models.performance_model.IO should be constant");

  for(int i=0; i<(int)vals.size(); i++)
  {
    if(!Param::getInstance()->getString(names[i]+".mode").compare("const"))
    {
      vals[i]->mode = PPC_CONST;
      vals[i]->val_1 = Param::getInstance()->getDouble(names[i]+".const_val");
    }
    else if(!Param::getInstance()->getString(names[i]+".mode").compare("normal"))
    {
      vals[i]->mode = PPC_NORMAL;
      vals[i]->val_1 = Param::getInstance()->getDouble(names[i]+".val_1");
      vals[i]->val_2 = Param::getInstance()->getDouble(names[i]+".val_2");
    }
    else if(!Param::getInstance()->getString(names[i]+".mode").compare("exponential"))
    {
      vals[i]->mode = PPC_EXP;
      vals[i]->val_1 = Param::getInstance()->getDouble(names[i]+".val_1");
      vals[i]->val_2 = Param::getInstance()->getDouble(names[i]+".val_2");
    }
    else if(!Param::getInstance()->getString(names[i]+".mode").compare("min_max"))
    {
      vals[i]->mode = PPC_MINMAX;
      vals[i]->val_1 = Param::getInstance()->getDouble(names[i]+".val_1");
      vals[i]->val_2 = Param::getInstance()->getDouble(names[i]+".val_2");
    }
    else if(!Param::getInstance()->getString(names[i]+".mode").compare("bimodal_addr"))
    {
      if(names[i].compare("ppc_models.performance_model.TON") &&
          names[i].compare("ppc_models.performance_model.TIN"))
        ERROR("Bimodal address based ditribution must only be used for TON or TIN");
      vals[i]->mode = PPC_ADDR;
      vals[i]->val_1 = Param::getInstance()->getDouble(names[i]+".val_1");
      vals[i]->val_2 = Param::getInstance()->getDouble(names[i]+".val_2");
    }
    else
      ERROR("Bad mode (" +  Param::getInstance()->getString(names[i]+".mode") +") for " + names[i]);
  }

  for(int i=0; i<(int)vals.size(); i++)
    if(vals[i]->mode != PPC_ADDR && vals[i]->mode != PPC_CONST)
      _randomSeed = Param::getInstance()->getInt("ppc_models.random_seed");

  _randomGen = gsl_rng_alloc(gsl_rng_rand48);
  gsl_rng_set(_randomGen, _randomSeed);

}

PpcGlobals::~PpcGlobals()
{
  gsl_rng_free(_randomGen);
}

PpcGlobals* PpcGlobals::getInstance()
{
  if(_singleton == NULL)
    _singleton = new PpcGlobals();

  return _singleton;
}

void PpcGlobals::kill()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

double PpcGlobals::ton(int offset)
{
  double res = -1;
  switch(_ton.mode)
  {
    case PPC_CONST:
      res = _ton.val_1;
      break;

    case PPC_NORMAL:
      res = RanDist::getInstance()->normalDist(_ton.val_1, _ton.val_2);
      break;

    case PPC_EXP:
      res = RanDist::getInstance()->exponentialDist(_ton.val_1, _ton.val_2);
      break;

    case PPC_MINMAX:
      res = RanDist::getInstance()->minMax(_ton.val_1, _ton.val_2);
      break;

    case PPC_ADDR:
      res = bimodalAddr(_ton.val_1, _ton.val_2, offset);
      break;

    default:
      ERROR("Bad mode for TON");
      break;
  }

  return res;
}

double PpcGlobals::tin(int offset)
{
  double res = -1;
  switch(_tin.mode)
  {
    case PPC_CONST:
      res = _tin.val_1;
      break;

    case PPC_NORMAL:
      res = RanDist::getInstance()->normalDist(_tin.val_1, _tin.val_2);
      break;

    case PPC_EXP:
      res = RanDist::getInstance()->exponentialDist(_tin.val_1, _tin.val_2);
      break;

    case PPC_MINMAX:
      res = RanDist::getInstance()->minMax(_tin.val_1, _tin.val_2);
      break;

    case PPC_ADDR:
      res = bimodalAddr(_tin.val_1, _tin.val_2, offset);
      break;

    default:
      ERROR("Bad mode for TIN");
      break;
  }

  return res;
}

double PpcGlobals::io()
{
  double res = -1;
   switch(_io.mode)
   {
     case PPC_CONST:
       res = _io.val_1;
       break;

     case PPC_NORMAL:
       res = RanDist::getInstance()->normalDist(_io.val_1, _io.val_2);
       break;

     case PPC_EXP:
       res = RanDist::getInstance()->exponentialDist(_io.val_1, _io.val_2);
       break;

     case PPC_MINMAX:
       res = RanDist::getInstance()->minMax(_io.val_1, _io.val_2);
       break;

     default:
       ERROR("Bad mode for IO");
       break;
   }

   return res;
}

double PpcGlobals::bers()
{
  double res = -1;
   switch(_bers.mode)
   {
     case PPC_CONST:
       res = _bers.val_1;
       break;

     case PPC_NORMAL:
       res = RanDist::getInstance()->normalDist(_bers.val_1, _bers.val_2);
       break;

     case PPC_EXP:
       res = RanDist::getInstance()->exponentialDist(_bers.val_1, _bers.val_2);
       break;

     case PPC_MINMAX:
       res = RanDist::getInstance()->minMax(_bers.val_1, _bers.val_2);
       break;

     default:
       ERROR("Bad mode for BERS");
       break;
   }

   return res;
}

double PpcGlobals::pton()
{
  double res = -1;
   switch(_pton.mode)
   {
     case PPC_CONST:
       res = _pton.val_1;
       break;

     case PPC_NORMAL:
       res = RanDist::getInstance()->normalDist(_pton.val_1, _pton.val_2);
       break;

     case PPC_EXP:
       res = RanDist::getInstance()->exponentialDist(_pton.val_1, _pton.val_2);
       break;

     case PPC_MINMAX:
       res = RanDist::getInstance()->minMax(_pton.val_1, _pton.val_2);
       break;

     default:
       ERROR("Bad mode for PTON");
       break;
   }

   return res;
}

double PpcGlobals::ptin()
{
  double res = -1;
   switch(_ptin.mode)
   {
     case PPC_CONST:
       res = _ptin.val_1;
       break;

     case PPC_NORMAL:
       res = RanDist::getInstance()->normalDist(_ptin.val_1, _ptin.val_2);
       break;

     case PPC_EXP:
       res = RanDist::getInstance()->exponentialDist(_ptin.val_1, _ptin.val_2);
       break;

     case PPC_MINMAX:
       res = RanDist::getInstance()->minMax(_ptin.val_1, _ptin.val_2);
       break;

     default:
       ERROR("Bad mode for PTIN");
       break;
   }

   return res;
}

double PpcGlobals::pio()
{
  double res = -1;
   switch(_pio.mode)
   {
     case PPC_CONST:
       res = _pio.val_1;
       break;

     case PPC_NORMAL:
       res = RanDist::getInstance()->normalDist(_pio.val_1, _pio.val_2);
       break;

     case PPC_EXP:
       res = RanDist::getInstance()->exponentialDist(_pio.val_1, _pio.val_2);
       break;

     case PPC_MINMAX:
       res = RanDist::getInstance()->minMax(_pio.val_1, _pio.val_2);
       break;

     default:
       ERROR("Bad mode for PIO");
       break;
   }

   return res;
}

double PpcGlobals::pbers()
{
  double res = -1;
   switch(_pbers.mode)
   {
     case PPC_CONST:
       res = _pbers.val_1;
       break;

     case PPC_NORMAL:
       res = RanDist::getInstance()->normalDist(_pbers.val_1, _pbers.val_2);
       break;

     case PPC_EXP:
       res = RanDist::getInstance()->exponentialDist(_pbers.val_1, _pbers.val_2);
       break;

     case PPC_MINMAX:
       res = RanDist::getInstance()->minMax(_pbers.val_1, _pbers.val_2);
       break;

     default:
       ERROR("Bad mode for PBERS");
       break;
   }

   return res;
}

double PpcGlobals::bimodalAddr(double min, double max, int offset)
{
  if(!(offset%2))
    return min;
  else
    return max;
}
