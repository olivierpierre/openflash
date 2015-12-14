#ifndef JFFS2_STATS_HPP
#define JFFS2_STATS_HPP

#include <vector>

using namespace std;

typedef enum
{
  JFFS2_READPAGE,
  JFFS2_WRITE_BEGIN,
  JFFS2_WRITE_END,
  JFFS2_OPEN,
  JFFS2_CREATE,
  JFFS2_SYNC,
  JFFS2_TRUNCATE,
  JFFS2_RENAME,
  JFFS2_CLOSE,
  JFFS2_GC_PASS
} jffs2_call_type_t;

typedef struct
{
  //double time;
  jffs2_call_type_t type;
  double process_time;
  double e_cpu;
  double e_mem;
} jffs2_call_timing_info_t;

class Jffs2Stats
{
public:
  Jffs2Stats(bool active) : _active(active) {};
  void addJffs2CallTimingInfo(jffs2_call_type_t type, double process_time, double e_cpu, double e_mem);
  vector<jffs2_call_timing_info_t> &getCallTimingInfos() {return _calls_timing_infos;};

private:
  bool _active;
  vector<jffs2_call_timing_info_t> _calls_timing_infos;
};

#endif /* JFFS2_STATS_HPP */
