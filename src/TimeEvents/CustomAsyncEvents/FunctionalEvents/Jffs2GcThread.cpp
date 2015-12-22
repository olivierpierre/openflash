#include "Jffs2GcThread.hpp"

#include "../../../FunctionalLayer/FunctionalLayer.hpp"
#include "../../EventProcessor.hpp"
#include "../../../Globs/RanDist.hpp"

Jffs2GcThread::Jffs2GcThread (double wake_time_from_now) : AsyncEvent(wake_time_from_now, true, 0.0, 0){}

int Jffs2GcThread::processEvent ()
{
#ifdef VERBOSE
  cout << "[Jffs2GCThread :" << TIME_NOW << "] " << "tick" << endl;
#endif /* VERBOSE */

  Jffs2 *jffs2 = static_cast<Jffs2 *>(Jffs2::getInstance());
  PpcValF res = jffs2->_gc->garbageCollectorPass();
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  jffs2->_gc->gcdDone();

  return 0;
}

void Jffs2GcThread::insert ()
{
  /* compute arrival time */
//  double rate = Param::getInstance()->getDouble("functional_model.ffs.jffs2.bg_thread_inter_arrival_exponential_rate");
  double base = Param::getInstance()->getDouble("functional_model.ffs.jffs2.bg_thread_inter_arrival_exponential_base");
//  double arrival_time = RanDist::getInstance()->exponentialRateDist(base, rate);
  double arrival_time = base;

  Jffs2GcThread *event = new Jffs2GcThread(arrival_time);
  EventProcessor::getInstance()->insertAsyncEventIn(*event);
  delete event;
}
