#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <vector>
#include <string>

#include "../Globs/Simulation.hpp"

using namespace std;

class Output
{
public:
  Output(string output_file_path, Simulation &simu);
  Output(Simulation &simu);
  virtual ~Output()=0;
  virtual int output()=0;
  virtual string getName()=0;

protected:
  string _output_file_path;
  Simulation &_simu;
};

class OutputPool
{
public:
  static OutputPool *getInstance();
  static void kill();
  int output();

private:
  vector<Output *> _outputs;
  // singleton stuff
  static OutputPool *_singleton;
  OutputPool();
  ~OutputPool();

};

#endif /* OUTPUT_HPP */
