#include "Event.hpp"

#include <assert.h>
#include <cstdlib>

#include "../Globs/Common.hpp"
#include "GlobalTime.hpp"
#include "../FunctionalLayer/FunctionalLayer.hpp"

/****** Abstract model *******************************************************/

Event::Event (double arrival_time)
{
  _is_processed = false;
  _arrival_time = arrival_time;
  _processing_start_time = 0;
  _res.time = _res.power_consumption = 0;
}

Event::~Event(){}

double Event::getProcessingTime ()
{
  assert(_is_processed);
  return _res.time;
}

double Event::getResponseTime ()
{
  assert(_is_processed);
  return _res.time + (_processing_start_time - _arrival_time);
}

int Event::process()
{
  _processing_start_time = TimeMgr::getInstance()->getTime();

  int res = processEvent();

  _is_processed = true;

  return res;
}

void Event::printRes()
{
  if(!_is_processed)
    ERROR("Calling printRes on event not processed yet");

  string event_type;
  if(getType() == WORKLOAD_EVENT)
    event_type = "workload";
  else
    event_type = "async";

  cout << "[EVTMGR:" << TIME_NOW << "] Processed " << event_type
      << " event arrived at " << _arrival_time << ", started:" << _processing_start_time <<
      ", time taken " << _res.time <<
      ", rtime:" << getResponseTime() << endl;
}

WorkloadEvent::~WorkloadEvent() {}

/****** Fake debug events *******************************************************/

int FakeWLEvent::processEvent ()
{
  _res.time = _forced_duration;
  _res.power_consumption = rand()%50;
  return 0;
}

int FakeAsyncEvent::processEvent ()
{
  _res.time = _forced_duration;
  _res.power_consumption = rand()%50;

  return 0;
}

/****** FFS events *******************************************************/


int VFSRead::processEvent()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsRead(_inode, _offset, _count);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSCreate::processEvent()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsCreate(_inode, _name_len);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSSync::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsSync();
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;

  return 0;
}

int VFSWrite::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsWrite(_inode, _offset, _count);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSRemove::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsRemove(_inode);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSRename::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsRename(_inode, _new_name_len);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSOpen::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsOpen(_inode, _truncate, _create, _name_len);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSTruncate::processEvent ()
{
  if(_length != 0)
    ERROR("Truncate to size != 0 not implemented yet");
  PpcValF res = VirtualFileSystem::getInstance()->vfsTruncate(_inode, _length);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSClose::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsClose(_inode);
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

int VFSDropCache::processEvent ()
{
  PpcValF res = VirtualFileSystem::getInstance()->vfsDropCache();
  _res.time = res.time;
  _res.power_consumption = res.e_cpu + res.e_mem;
  return 0;
}

/****** FTL events *******************************************************/


int FTLSysRead::processEvent ()
{
  _res = FtlSystem::getInstance()->ftlSysRead(_addr, _size);
  return 0;
}

int FTLSysWrite::processEvent ()
{
  _res = FtlSystem::getInstance()->ftlSysWrite(_addr, _size);
  return 0;
}

int FTLSysTrim::processEvent ()
{
  _res = FtlSystem::getInstance()->ftlSysTrim(_addr, _size);
  return 0;
}
