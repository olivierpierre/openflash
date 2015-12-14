#ifndef EVENT_PROCESSOR_HPP
#define	EVENT_PROCESSOR_HPP

#include "GlobalTime.hpp"
#include "Event.hpp"

class EventProcessor
{
public:
  static EventProcessor *getInstance();
  static void kill();

  int insertWorkloadEvent(Event &e);
  int insertAsyncEvent(AsyncEvent &e);
  int insertAsyncEventIn(AsyncEvent &e);

private:
  EventProcessor();
  ~EventProcessor();
  vector<AsyncEvent *> _async_events_queue;
  TimeMgr *_time;
  double _last_wl_event_arrival_time;

  vector<AsyncEvent *> _async_events_to_insert;

  int getNextAsyncEventIndex(double free_time);
  int removeEventFromAEList(int index);

  static EventProcessor *_singleton;
};

#endif /* EVENT_PROCESSOR_HPP */
