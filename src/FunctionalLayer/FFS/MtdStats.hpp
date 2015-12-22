#ifndef MTD_STATS_HPP
#define MTD_STATS_HPP

#include <vector>
#include <stdint.h>
#include "../../FlashLayer/Address.hpp"

using namespace std;

typedef enum
{
	MTD_SRC_JFFS2_READPAGE,
	MTD_SRC_JFFS2_WRITE_BEGIN,
	MTD_SRC_JFFS2_WRITE_END,
	MTD_SRC_JFFS2_GC,
	MTD_SRC_JFFS2_REMOVE,
	MTD_SRC_JFFS2_CREATE,
	MTD_SRC_JFFS2_SYNC,
	MTD_SRC_JFFS2_TRUNCATE,
	MTD_SRC_JFFS2_RENAME,
	MTD_SRC_UNKNOWN
} mtd_call_src_t;

typedef enum
{
  MTD_READ,
  MTD_READ_HIT,
  MTD_WRITE,
  MTD_ERASE
} mtd_call_type_t;

typedef struct
{
  mtd_call_type_t type;
  uint64_t page_index;
  double process_time;
  double energy_mem;
  double energy_cpu;
  mtd_call_src_t source;
} mtd_call_timing_info_t;

class MtdStats
{
public:
  MtdStats(bool active) : _active(active) {};
  void addMtdCallInfo(mtd_call_type_t type, uint64_t page_index, double process_time,
                      double energy_mem, double energy_cpu, mtd_call_src_t src=MTD_SRC_UNKNOWN);
  vector<mtd_call_timing_info_t> &getCallTimingInfos() {return _calls_timing_infos;};
  void flushStats() {_calls_timing_infos.clear();}

private:
  bool _active;
  vector<mtd_call_timing_info_t> _calls_timing_infos;
};

#endif /* MTD_STATS_HPP */
