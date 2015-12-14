#ifndef JFFS2_GC_THREAD_HPP
#define JFFS2_GC_THREAD_HPP

#include "../../Event.hpp"

class Jffs2GcThread : public AsyncEvent
{
public:
  Jffs2GcThread (double wake_time_from_now);
  Jffs2GcThread *clone() const { return new Jffs2GcThread(*this); };
  static void insert();

private:
  int processEvent();


  friend class Jffs2Gc;
};



#endif /* JFFS2_GC_THREAD_HPP */
