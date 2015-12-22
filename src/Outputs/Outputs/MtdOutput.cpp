#include "MtdOutput.hpp"

#include <iostream>
#include <fstream>

int MtdOutput::output ()
{
  streambuf * buf;
  ofstream of;
  if(_output_file_path.compare("")) {
      of.open(_output_file_path.c_str());
      buf = of.rdbuf();
  } else {
      buf = std::cout.rdbuf();
  }
  std::ostream out(buf);

  // TODO is the erase adress a block or a page
  out << "# MTD Statistics" << endl;
  out << "# ==============" << endl;
  out << "# operation;address (page index for R/W, bloc for E);time taken;energy mem;energy cpu" << endl;

  /* mtdcalls process times */
  vector<mtd_call_timing_info_t> &res = _mtd_stats->getCallTimingInfos();
  for(int i=0; i<(int)res.size(); i++)
    if(res[i].type == MTD_READ)
      out << "mtd_read;" << res[i].page_index << ";" << res[i].process_time << ";" << res[i].energy_mem << ";" << res[i].energy_cpu << ";" << getSource(res[i])<< endl;
    else if(res[i].type == MTD_WRITE)
      out << "mtd_write;" <<  res[i].page_index << ";" << res[i].process_time << ";" << res[i].energy_mem << ";" << res[i].energy_cpu << ";" << getSource(res[i])<< endl;
    else if(res[i].type == MTD_ERASE)
      out << "mtd_erase;" <<  res[i].page_index << ";" << res[i].process_time << ";" << res[i].energy_mem << ";" << res[i].energy_cpu << ";" << getSource(res[i])<< endl;
    else if(res[i].type == MTD_READ_HIT)
      out << "mtd_read_hit;" <<  res[i].page_index << ";" << res[i].process_time << ";" << res[i].energy_mem << ";" << res[i].energy_cpu << ";" << getSource(res[i])<< endl;
    else
      ERROR("[Mtd Ouput] Not implemented yet");


  if(of.is_open())
      of.close();

  return 0;
}

string MtdOutput::getSource(mtd_call_timing_info_t obj)
{
	string res = "src_unknown";

	switch(obj.source)
	{
	case MTD_SRC_JFFS2_READPAGE:
		res = "jffs2_readpage";
		break;
	case MTD_SRC_JFFS2_WRITE_BEGIN:
		res = "jffs2_write_begin";
		break;
	case MTD_SRC_JFFS2_WRITE_END:
		res = "jffs2_write_end";
		break;
	case MTD_SRC_JFFS2_GC:
		res = "jffs2_gc";
		break;
	case MTD_SRC_JFFS2_REMOVE:
		res = "jffs2_remove";
		break;
	case MTD_SRC_JFFS2_CREATE:
		res = "jffs2_create";
		break;
	case MTD_SRC_JFFS2_SYNC:
		res = "jffs2_sync";
		break;
	case MTD_SRC_JFFS2_TRUNCATE:
		res = "jffs2_truncate";
		break;
	case MTD_SRC_JFFS2_RENAME:
		res = "jffs2_rename";
		break;
	case MTD_SRC_UNKNOWN:
		break;
	}

	return res;
}
