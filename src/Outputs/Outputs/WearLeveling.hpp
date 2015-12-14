#ifndef WEAR_LEVELING_HPP
#define WEAR_LEVELING_HPP

#include "../Output.hpp"
#include "../../FlashLayer/FlashSystem.hpp"

class WearLevelingOutput : public Output
{
public:
  WearLevelingOutput(string output_file, Simulation &simu) : Output(output_file, simu) {};
  WearLevelingOutput(Simulation &simu) : Output(simu) {};
  ~WearLevelingOutput() {};

  string getName() {return "Wear leveling output [" + _output_file_path + "]";};
  int output();


private:

  uint32_t getTotalNumberOfEraseOperations(FlashSystem &f, bool count_legacy=true, bool count_multi_plane=true);
};

#endif /* WEAR_LEVELING_HPP */
