#ifndef JFFS2_OUTPUT_HPP
#define JFFS2_OUTPUT_HPP

#include "../Output.hpp"



class Jffs2Output : public Output
{
public:
  Jffs2Output(string output_file, Simulation &simu, Jffs2Stats *jffs2_stats) :
    Output(output_file, simu), _jffs2_stats(jffs2_stats) {};
  Jffs2Output(Simulation &simu, Jffs2Stats *jffs2_stats) :
  Output(simu), _jffs2_stats(jffs2_stats) {};
  ~Jffs2Output() {};

  string getName() {return "Jffs2 output [" + _output_file_path + "]";};
  int output();

private:
  Jffs2Stats *_jffs2_stats;
};

#endif /* JFFS2_OUTPUT_HPP */
