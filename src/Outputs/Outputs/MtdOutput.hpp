#ifndef MTD_OUTPUT_HPP
#define MTD_OUTPUT_HPP

#include "../Output.hpp"

class MtdOutput : public Output
{
public:
  MtdOutput(string output_file, Simulation &simu, MtdStats *mtd_stats) :
    Output(output_file, simu), _mtd_stats(mtd_stats) {};
  MtdOutput(Simulation &simu, MtdStats *mtd_stats) :
  Output(simu), _mtd_stats(mtd_stats) {};
  ~MtdOutput() {};

  string getName() {return "Mtd output [" + _output_file_path + "]";};
  int output();

private:
  MtdStats *_mtd_stats;

};

#endif /* MTD_OUTPUT_HPP */
