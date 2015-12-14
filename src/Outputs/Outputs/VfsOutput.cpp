#include "VfsOutput.hpp"

#include <iostream>
#include <fstream>

int VfsOutput::output ()
{
  streambuf * buf;
  ofstream of;
  if(_output_file_path.compare("")) {
      of.open(_output_file_path.c_str());
      buf = of.rdbuf();
  } else {
      buf = std::cout.rdbuf();
  }
  std::ostream out(buf);

  /* get stats */
  vector<vfs_call_timing_info_t> &res = _vfs_stats->getCallTimingInfos();

  out << "# VFS Statistics" << endl;
  out << "# ================" << endl;
  out << "# ---------------------------" << endl;
  out << "# Total energy at VFS level (J) : " << endl;
  out << "# ---------------------------" << endl;
  out << "#  - CPU : " << _vfs_stats->getTotalCpuEnergy() << endl;
  out << "#  - Mem : " << _vfs_stats->getTotalMemEnergy() << endl;
  out << "# ---------------------------" << endl;
  out << "# Mean energy per syscall (J) : " << endl;
  out << "# -------------------------" << endl;
  out << "#  - CPU : " << _vfs_stats->getMeanCpuEnergyPerSysCall() << endl;
  out << "#  - Mem : " << _vfs_stats->getMeanMemEnergyPerSysCall() << endl;
  out << "# ---------------------------" << endl;
  out << "# Mean exec. time per syscall (s) : " << endl;
  out << "# -------------------------" << endl;
  out << "# " << _vfs_stats->getMeanTimePerSysCall()/1000000.0 << endl;
  out << "# ---------------------------" << endl;
  out << "# Total time at VFS level (s) : " << endl;
  out << "# -------------------------" << endl;
  out << "# " << _vfs_stats->getTotalTime()/1000000.0 << endl;

  out << "# --------------------------------------" << endl;
  out << "# Detailed timing / energy per syscall: " << endl;
  out << "# ---------------------------------------" << endl;
  out << "# type ; time ; e_mem ; e_cpu" << endl;

  /* system calls process times */
  for(int i=0; i<(int)res.size(); i++)
    if(res[i].type == VFS_READ)
      out << "vfs_read;" << res[i].process_time << ";" << res[i].e_mem <<";" << res[i].e_cpu << endl;
    else if(res[i].type == VFS_WRITE)
      out << "vfs_write;" << res[i].process_time << ";" << res[i].e_mem <<";" << res[i].e_cpu << endl;
    else
      ERROR("[VFS Ouput] Not implemented yet");


  if(of.is_open())
      of.close();

  return 0;
}
