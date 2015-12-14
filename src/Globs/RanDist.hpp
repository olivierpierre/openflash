#ifndef RANDIST_HPP
#define RANDIST_HPP

#include <iostream>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

using namespace std;

class RanDist
{
public:
  static RanDist* getInstance();
  static void kill();

  double normalDist(double mean, double stdev);
  double exponentialDist(double base, double mean);
  double exponentialRateDist(double base, double rate);
  double minMax(double min, double max);
  double bimodalAddr(double min, double max, int offset);

private:

  gsl_rng *_gsl_rng;
  int _random_seed;

  RanDist();
  ~RanDist();
  static RanDist *_singleton;
};

#endif /* RANDIST_HPP */
