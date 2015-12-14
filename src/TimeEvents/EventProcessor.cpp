#include "EventProcessor.hpp"

#include <assert.h>



EventProcessor *EventProcessor::_singleton = NULL;

EventProcessor::EventProcessor()
{
  _time = TimeMgr::getInstance();
  _last_wl_event_arrival_time = 0;
}

EventProcessor* EventProcessor::getInstance()
{
  if(_singleton == NULL)
    _singleton = new EventProcessor();

  return _singleton;
}

EventProcessor::~EventProcessor()
{
}

int EventProcessor::insertWorkloadEvent (Event& e)
{
  double free_time = e.getArrivalTime() - TIME_NOW;
  int ret;

  // check the wl events are correctly ordered
  if(e.getArrivalTime() < _last_wl_event_arrival_time)
    ERROR("Current WL Event arrival time " + nts(e.getArrivalTime()) +
          " < last WL event arrival time " + nts(_last_wl_event_arrival_time));
  _last_wl_event_arrival_time = e.getArrivalTime();

  // First look at the async event list
  while(1)
  {
    int async_event_index = getNextAsyncEventIndex(free_time);
    if(async_event_index == -1)
      break;

    AsyncEvent *ae = _async_events_queue[async_event_index];

     //jump to the ae event arrival time if needed
    if(ae->getArrivalTime() > TIME_NOW)
    {
      free_time -= ae->getArrivalTime() - TIME_NOW;
      _time->setTime(ae->getArrivalTime());
    }

    //if there is a min timeout on the async event to process
    _time->incrTime(ae->getMinTimeOut());

    // process
    if((ret = ae->process()))
      ERROR("Error processing event, returned " + nts(ret));

    free_time -= ae->getProcessingTime();
    _time->incrTime(ae->getProcessingTime());

#ifdef VERBOSE
    ae->printRes();
#endif /* VERBOSE */

    // if the event is periodic
    // FIXME for now the frequency represents the time between the end of an occurence of the event
    // and the start of the next one. As the start of the first occurence can be delayed, and the fact
    // that the first event has itself a processign time, the frequency is not strictly (not at all in some
    // heavy worloads cases) the time between to starts of the periodic event
    if(ae->getFrequency())
    {
      AsyncEvent *ae2 = ae->clone();
      ae2->setArrivalTime(TIME_NOW + ae->getFrequency());
      insertAsyncEvent(*ae2);
      delete ae2;
    }

    removeEventFromAEList(async_event_index);
  }

  //Now process Workload request

  // jump to arrival time if we are not here yet
  if(_time->getTime() < e.getArrivalTime())
    _time->setTime(e.getArrivalTime());

  // now process
  e.process();
  _time->incrTime(e.getProcessingTime());

#ifdef VERBOSE
  e.printRes();
#endif /* VERBOSE */

  /* insert events to insert */
  if(!_async_events_to_insert.empty())
    for(int i=0; i<(int)_async_events_to_insert.size();i++)
    {
      _async_events_to_insert[i]->setArrivalTime(_time->getTime() + _async_events_to_insert[i]->getArrivalTime());
      insertAsyncEvent(*(_async_events_to_insert[i]));	// will clone
      delete _async_events_to_insert[i];
    }
  _async_events_to_insert.clear();


  return 0;
}

/**
 * Insert an asynchronous event in the async queue, _in order_ so that
 * _async_events_queue.begin() will be the next event to execute
 */
int EventProcessor::insertAsyncEvent (AsyncEvent& e)
{
  bool inserted = false;

  if(e.getType() != ASYNC_EVENT)
    ERROR("Inserting as async event something different of async event type");

  AsyncEvent *e_ptr = static_cast<AsyncEvent *>(e.clone());

  if(e.getArrivalTime() < _time->getTime())
    ERROR("Inserting an asynchronous event in the past ...");

  for(int i=0; i<(int)_async_events_queue.size(); i++)
    if(_async_events_queue[i]->getArrivalTime() > e_ptr->getArrivalTime())
    {
      _async_events_queue.insert(_async_events_queue.begin()+i, e_ptr);
      inserted = true;
      break;
    }

  if(!inserted)
    _async_events_queue.push_back(e_ptr);

#ifdef VERBOSE
  cout << "[EVTMGR:" << TIME_NOW << "] Added new async. event arriving at " << e_ptr->getArrivalTime() << " to the queue" << endl;
#endif /* VERBOSE */

  return 0;
}

void EventProcessor::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

/**
 * Return the next async event to process in the free_time period
 * We look at the async event queue in which events are sorted by arrival
 * time. We also must take into account the min timeout for each async event :
 * for exemple if an event arived at T has a min timeout of 100 and the free time is 90
 * that event cannot be launched. But maybe there is an event arrived at T+1
 * with no timeout that can be launched
 * returns -1 if no async event can be launched
 */
int EventProcessor::getNextAsyncEventIndex(double free_time)
{
  if(_async_events_queue.empty())	// no async event in queue
    return -1;

  if(_async_events_queue[0]->getArrivalTime() > (TIME_NOW + free_time))	// Dont process events in the future
    return -1;

  for(int i=0; i<(int)_async_events_queue.size(); i++)
  {
    AsyncEvent *ae = static_cast<AsyncEvent *>(_async_events_queue[i]);

    if(ae->getPriorityOverWL())	// priority over WL req, don't care about free time
      return i;

    if(ae->getArrivalTime() >= TIME_NOW + free_time)	// could not find any event in the free time slice
      return -1;

    if(ae->getMinTimeOut() < free_time)
      return i;
  }

  return -1;
}

int EventProcessor::removeEventFromAEList (int index)
{
  delete _async_events_queue[index];
  _async_events_queue.erase(_async_events_queue.begin() + index);

  return 0;
}

/**
 * Insert an asynchronous event at a the time == getTime() + e.arrival_time
 */
int EventProcessor::insertAsyncEventIn (AsyncEvent& e)
{
  if(e.getType() != ASYNC_EVENT)
      ERROR("Inserting as async event something different of async event type");

    AsyncEvent *e_ptr = static_cast<AsyncEvent *>(e.clone());

    _async_events_to_insert.push_back(e_ptr);

    return 0;
}
