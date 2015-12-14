#ifndef MTD_STATS_HPP
#define MTD_STATS_HPP

#include <vector>
#include <stdint.h>
#include "../../FlashLayer/Address.hpp"

using namespace std;

typedef enum
{
  MTD_READ,
  MTD_READ_HIT,
  MTD_WRITE,
  MTD_ERASE
} mtd_call_type_t;

typedef struct
{
  mtd_call_type_t type;
  uint64_t page_index;
  double process_time;
  double energy_mem;
  double energy_cpu;
} mtd_call_timing_info_t;

class MtdStats
{
public:
  MtdStats(bool active) : _active(active) {};
  void addMtdCallInfo(mtd_call_type_t type, uint64_t page_index, double process_time,
                      double energy_mem, double energy_cpu);
  vector<mtd_call_timing_info_t> &getCallTimingInfos() {return _calls_timing_infos;};

private:
  bool _active;
  vector<mtd_call_timing_info_t> _calls_timing_infos;
};

#endif /* MTD_STATS_HPP */
