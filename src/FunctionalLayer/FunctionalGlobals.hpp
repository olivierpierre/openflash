#ifndef FUNCTIONAL_GLOBALS_HPP
#define FUNCTIONAL_GLOBALS_HPP

#include <iostream>

using namespace std;

class PpcValF
{
public:
  double time;
  double e_cpu;
  double e_mem;

  friend ostream& operator<<(ostream& os, const PpcValF& val);  /**< print on stream */

};

PpcValF operator+(const PpcValF& a, const PpcValF& b);

class PpcValF2
{
public:
  double time;
  double power_consumption;

  friend ostream& operator<<(ostream& os, const PpcValF2& val);  /**< print on stream */

};

PpcValF2 operator+(const PpcValF2& a, const PpcValF2& b);

#endif /* FUNCTIONAL_GLOBALS_HPP */
