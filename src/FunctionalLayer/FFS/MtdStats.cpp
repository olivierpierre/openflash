#include "MtdStats.hpp"
#include "../../TimeEvents/GlobalTime.hpp"

void MtdStats::addMtdCallInfo (mtd_call_type_t type, uint64_t page_index,
				    double process_time, double energy_mem, double energy_cpu,
					mtd_call_src_t src)
{
  if(!_active)
    return;

  mtd_call_timing_info_t info;

  info.type = type;
  info.page_index = page_index;
  info.process_time = process_time;
  info.energy_mem = energy_mem;
  info.energy_cpu = energy_cpu;
  info.source = src;

  _calls_timing_infos.push_back(info);
}
