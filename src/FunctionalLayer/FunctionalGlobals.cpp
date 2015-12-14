#include "FunctionalGlobals.hpp"

PpcValF operator+(const PpcValF& a, const PpcValF& b)
{
  PpcValF res;

  res.time = a.time + b.time;
  res.e_mem = a.e_mem + b.e_mem;
  res.e_cpu = a.e_cpu + b.e_cpu;

  return res;
}

ostream& operator<<(ostream& os, const PpcValF &val)
{
    os << "PpcValF t:" << val.time << ", e_mem:" << val.e_mem << ", e_cpu:" << val.e_cpu;
    return os;
}

PpcValF2 operator+(const PpcValF2& a, const PpcValF2& b)
{
  PpcValF2 res;

  res.time = a.time + b.time;
  res.power_consumption = a.power_consumption + b.power_consumption;

  return res;
}

ostream& operator<<(ostream& os, const PpcValF2 &val)
{
    os << "PpcValF t:" << val.time << ", power con.:" << val.power_consumption;
    return os;
}
