#include "Jffs2Output.hpp"

#include <iostream>
#include <fstream>

int Jffs2Output::output ()
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

  out << "# JFFS2 Statistics" << endl;
  out << "# ================" << endl;
  out << "# A call to jffs2_readpage can be triggered by a vfs_read with a page cache miss, but also a " << endl;
  out << "# call to jffs2_write_begin (coming from a vfs_write) on a page wich is not up to date. " << endl;
  out << "# type ; process time ; e_mem ; e_cpu" << endl;

  /* fs system calls process times */
  vector<jffs2_call_timing_info_t> &res = _jffs2_stats->getCallTimingInfos();
  for(int i=0; i<(int)res.size(); i++)
    if(res[i].type == JFFS2_READPAGE)
      out << "jffs2_readpage;" << res[i].process_time << ";" << res[i].e_mem << ";" << res[i].e_cpu << endl;
    else if(res[i].type == JFFS2_WRITE_BEGIN)
      out << "jffs2_write_begin;" << res[i].process_time << ";" << res[i].e_mem << ";" << res[i].e_cpu << endl;
    else if(res[i].type == JFFS2_WRITE_END)
      out << "jffs2_write_end;" << res[i].process_time << ";" << res[i].e_mem << ";" << res[i].e_cpu << endl;
    else if(res[i].type == JFFS2_GC_PASS)
      out << "jffs2_garbage_collect_pass" << ";" << res[i].process_time << ";"
	<< res[i].e_mem << ";" << res[i].e_cpu << endl;
    else
      ERROR("[Jffs2 Ouput] Not implemented yet");


  if(of.is_open())
      of.close();

  return 0;
}
