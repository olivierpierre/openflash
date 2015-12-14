#include "PdFlush.hpp"

#include "../../../FunctionalLayer/FunctionalLayer.hpp"

PdFlush::PdFlush (double frequency) : AsyncEvent(0.0, true, 0.0, frequency){}

int PdFlush::processEvent ()
{
#ifdef VERBOSE
  cout << "[PdFlush:" << TIME_NOW << "] " << "tick" << endl;
#endif /* VERBOSE */
  PpcValF res = VirtualFileSystem::getInstance()->vfsSync();
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}
