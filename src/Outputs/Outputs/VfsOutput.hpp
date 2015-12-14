#ifndef VFS_OUTPUT_HPP
#define VFS_OUTPUT_HPP

#include "../Output.hpp"
#include "../../FunctionalLayer/FFS/VFS/VfsStats.hpp"



class VfsOutput : public Output
{
public:
  VfsOutput(string output_file, Simulation &simu, VfsStats *vfs_stats) :
    Output(output_file, simu), _vfs_stats(vfs_stats) {};
  VfsOutput(Simulation &simu, VfsStats *vfs_stats) :
  Output(simu), _vfs_stats(vfs_stats) {};
  ~VfsOutput() {};

  string getName() {return "VFS output [" + _output_file_path + "]";};
  int output();

private:
  VfsStats *_vfs_stats;
};

#endif /* VFS_OUTPUT_HPP */
