#include "Jffs2Stats.hpp"
#include "../../../TimeEvents/GlobalTime.hpp"

void Jffs2Stats::addJffs2CallTimingInfo (jffs2_call_type_t type,
				    double process_time, double e_cpu, double e_mem)
{
  if(!_active)
    return;

  jffs2_call_timing_info_t info;

  info.type = type;
  info.process_time = process_time;

  _calls_timing_infos.push_back(info);
}
