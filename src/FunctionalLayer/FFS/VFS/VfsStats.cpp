#include "VfsStats.hpp"
#include "../../../TimeEvents/GlobalTime.hpp"

void VfsStats::addVfsCallInfo(vfs_call_type_t type,
				    double process_time, double e_cpu, double e_mem)
{
  if(!_active)
    return;

  vfs_call_timing_info_t info;

  info.type = type;
  info.process_time = process_time;
  info.e_cpu = e_cpu;
  info.e_mem = e_mem;

  _calls_timing_infos.push_back(info);
}

double VfsStats::getTotalMemEnergy ()
{
  double acc = 0;
  for(int i=0; i<(int)_calls_timing_infos.size(); i++)
    acc += _calls_timing_infos[i].e_mem;
  return acc;
}

double VfsStats::getTotalCpuEnergy ()
{
  double acc = 0;
  for(int i=0; i<(int)_calls_timing_infos.size(); i++)
    acc += _calls_timing_infos[i].e_cpu;
  return acc;
}

double VfsStats::getMeanCpuEnergyPerSysCall ()
{
  return getTotalCpuEnergy()/_calls_timing_infos.size();
}

double VfsStats::getMeanMemEnergyPerSysCall ()
{
  return getTotalMemEnergy()/_calls_timing_infos.size();
}

double VfsStats::getMeanTimePerSysCall ()
{
  return getTotalTime()/_calls_timing_infos.size();
}

double VfsStats::getTotalTime ()
{
  double acc = 0;
  for(int i=0; i<(int)_calls_timing_infos.size(); i++)
    acc += _calls_timing_infos[i].process_time;
  return acc;
}
