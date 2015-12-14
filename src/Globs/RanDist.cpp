#include "RanDist.hpp"
#include "Param.hpp"
#include "Common.hpp"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <cmath>

RanDist *RanDist::_singleton = NULL;

RanDist* RanDist::getInstance ()
{
  if(_singleton == NULL)
    _singleton = new RanDist();

  return _singleton;
}

void RanDist::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

RanDist::RanDist ()
{
  _random_seed = Param::getInstance()->getInt("simulator.random_seed");
  _gsl_rng = gsl_rng_alloc(gsl_rng_rand48);
  gsl_rng_set(_gsl_rng, _random_seed);
}

RanDist::~RanDist ()
{
  gsl_rng_free(_gsl_rng);
}

double RanDist::normalDist(double mean, double stdev)
{
  return  gsl_ran_gaussian(_gsl_rng, stdev) + mean;
}

double RanDist::exponentialDist(double base, double mean)
{
  return gsl_ran_exponential(_gsl_rng, mean) + base;
}

double RanDist::minMax(double min, double max)
{
  return gsl_ran_flat(_gsl_rng, min, max);
}

double RanDist::exponentialRateDist (double base, double rate)
{
  double mean = pow(rate, -1);
  return exponentialDist(base, mean);
}

double RanDist::bimodalAddr (double min, double max, int offset)
{
  ERROR("Not implemented yet");
  return -1;
}

