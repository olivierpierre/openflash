#ifndef EVENT_HPP
#define EVENT_HPP

#include <stdint.h>

#include "../FunctionalLayer/FunctionalGlobals.hpp"

typedef enum
{
  WORKLOAD_EVENT,
  ASYNC_EVENT
} event_type_t;

/******************** Abstract model ************************************/

class Event
{
public:
  Event(double arrival_time);
  virtual ~Event();

  int process();
  double getProcessingTime();
  double getResponseTime();
  double getArrivalTime() {return _arrival_time;};
  void setArrivalTime(double arrival_time) {_arrival_time = arrival_time;};
  virtual event_type_t getType()=0;

  //debug
  void printRes();

protected:
  bool _is_processed;
  double _arrival_time;
  double _processing_start_time;
  PpcValF2 _res;
  // response time = duration + (processing_start_time - arrival_time)

  virtual int processEvent()=0;
};

class WorkloadEvent : public Event
{
public:
  WorkloadEvent(double arrival_time) : Event(arrival_time) {};
  event_type_t getType() {return WORKLOAD_EVENT;};
  virtual ~WorkloadEvent()=0;
};


class AsyncEvent : public Event
{
public:
  AsyncEvent(double arrival_time, bool priority_over_wl, double min_timeout, double frequency) :
    Event(arrival_time), _priority_over_wl_request(priority_over_wl), _min_timeout(min_timeout),
    _frequency(frequency){};

  void setArrivalTime(double arrival_time) {_arrival_time = arrival_time;};

  bool getPriorityOverWL() {return _priority_over_wl_request;};
  double getMinTimeOut() {return _min_timeout;};
  double getFrequency() {return _frequency;};

  event_type_t getType() {return ASYNC_EVENT;};

  virtual AsyncEvent *clone () const = 0;

protected:

  bool _priority_over_wl_request;	// will be executed ASAP even if it delays a WL req arrived sooner
  double _min_timeout;			// amount of free time before it can be launched
  double _frequency;			// for periodic async events
};

/******************** Debug fake events ************************************/

class FakeAsyncEvent : public AsyncEvent
{
public:
  FakeAsyncEvent(double arrival_time, bool priority_over_wl, double min_timeout,
                 double frequency, double forced_duration) :
    AsyncEvent(arrival_time, priority_over_wl, min_timeout, frequency), _forced_duration(forced_duration) {};

  FakeAsyncEvent *clone() const { return new FakeAsyncEvent(*this); };

private:
  int processEvent();
  double _forced_duration;
};

class FakeWLEvent : public WorkloadEvent
{
public:
  FakeWLEvent(double arrival_time, double forced_duration) :
    WorkloadEvent(arrival_time), _forced_duration(forced_duration) {};

private:
  int processEvent();

  double _forced_duration;
};

/******************** FFS related events ************************************/

class VFSRead : public WorkloadEvent
{
public:
  VFSRead(double arrival_time, uint32_t inode, uint32_t offset, uint32_t count) :
    WorkloadEvent(arrival_time), _inode(inode), _offset(offset), _count(count) {};

private:
  uint32_t _inode;
  uint32_t _offset;
  uint32_t _count;

  int processEvent();
};

class VFSWrite : public WorkloadEvent
{
public:
  VFSWrite(double arrival_time, uint32_t inode, uint32_t offset, uint32_t count) :
    WorkloadEvent(arrival_time), _inode(inode), _offset(offset), _count(count) {};

private:
  uint32_t _inode;
  uint32_t _offset;
  uint32_t _count;

  int processEvent();
};

class VFSCreate : public WorkloadEvent
{
public:
  VFSCreate(double arrival_time, uint32_t inode, uint32_t name_len) :
    WorkloadEvent(arrival_time), _inode(inode), _name_len(name_len) {};

private:
  uint32_t _inode;
  uint32_t _name_len;

  int processEvent();
};

class VFSRemove : public WorkloadEvent
{
public:
  VFSRemove(double arrival_time, uint32_t inode) :
    WorkloadEvent(arrival_time), _inode(inode) {};
private:
  uint32_t _inode;

  int processEvent();
};

class VFSTruncate : public WorkloadEvent
{
public:
  VFSTruncate(double arrival_time, uint32_t inode, uint32_t length) :
    WorkloadEvent(arrival_time), _inode(inode), _length(length) {};
private:
  uint32_t _inode;
  uint32_t _length;

  int processEvent();
};

class VFSSync : public WorkloadEvent
{
public:
  VFSSync(double arrival_time) : WorkloadEvent(arrival_time) {};

private:
  int processEvent();
};

class VFSClose : public WorkloadEvent
{
public:
  VFSClose(double arrival_time, uint32_t inode) : WorkloadEvent(arrival_time), _inode(inode) {};

private:
  uint32_t _inode;

  int processEvent();
};

class VFSDropCache : public WorkloadEvent
{
public:
  VFSDropCache(double arrival_time) : WorkloadEvent(arrival_time) {};

private:
  int processEvent();
};

class VFSRename : public WorkloadEvent
{
public:
  VFSRename(double arrival_time, uint32_t inode, int new_name_len) :
    WorkloadEvent(arrival_time), _inode(inode), _new_name_len(new_name_len) {};

private:
  uint32_t _inode;
  int _new_name_len;

  int processEvent();
};

class VFSOpen : public WorkloadEvent
{
public:
  VFSOpen(double arrival_time, uint32_t inode, bool truncate, bool create, int name_len) :
    WorkloadEvent(arrival_time), _inode(inode), _truncate(truncate), _create(create), _name_len(name_len) {};

private:
  uint32_t _inode;
  bool _truncate;
  bool _create;
  int _name_len;

  int processEvent();
};

/******************** FTL related events ************************************/
// TODO

class FTLSysRead : public WorkloadEvent
{
public:
  FTLSysRead(double arrival_time, uint64_t addr, uint32_t size) :
    WorkloadEvent(arrival_time), _addr(addr), _size(size) {};

private:
  uint64_t _addr;
  uint32_t _size;

  int processEvent();
};

class FTLSysWrite : public WorkloadEvent
{
public:
  FTLSysWrite(double arrival_time, uint64_t addr, uint32_t size) :
    WorkloadEvent(arrival_time), _addr(addr), _size(size) {};

private:
  uint64_t _addr;
  uint32_t _size;

  int processEvent();
};

/**
 * TODO investigate the trim command to know how it can be modeled
 */
class FTLSysTrim : public WorkloadEvent
{
public:
  FTLSysTrim(double arrival_time, uint64_t addr, uint32_t size) :
    WorkloadEvent(arrival_time), _addr(addr), _size(size) {};

private:
  uint64_t _addr;
  uint32_t _size;

  int processEvent();
};
#endif /* EVENT_HPP */
