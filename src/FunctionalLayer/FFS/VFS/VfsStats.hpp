#ifndef VFS_STATS_HPP
#define VFS_STATS_HPP

#include <vector>

using namespace std;

typedef enum
{
  VFS_READ,
  VFS_WRITE,
  VFS_OPEN,
  VFS_REMOVE,
  VFS_CREATE,
  VFS_TRUNCATE,
  VFS_RENAME,
  VFS_CLOSE,
  VFS_SYNC,
  VFS_DROPCACHES
} vfs_call_type_t;

typedef struct
{
  //double time;
  vfs_call_type_t type;
  double process_time;
  double e_cpu;
  double e_mem;
} vfs_call_timing_info_t;

class VfsStats
{
public:
  VfsStats(bool active) : _active(active) {};
  void addVfsCallInfo(vfs_call_type_t type, double process_time, double e_cpu, double e_mem);
  vector<vfs_call_timing_info_t> &getCallTimingInfos() {return _calls_timing_infos;};
  double getTotalMemEnergy();
  double getTotalCpuEnergy();
  double getMeanCpuEnergyPerSysCall();
  double getMeanMemEnergyPerSysCall();
  double getMeanTimePerSysCall();
  double getTotalTime();
  /* TODO Etc. */

private:
  bool _active;
  vector<vfs_call_timing_info_t> _calls_timing_infos;
};

#endif /* VFS_STATS_HPP */
