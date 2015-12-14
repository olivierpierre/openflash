#include "WearLeveling.hpp"

#include <iostream>
#include <fstream>
#include "../../FlashLayer/FlashLayer.hpp"

using namespace std;

int WearLevelingOutput::output()
{
  FlashSystem &f = *(_simu._flash_layer); (void)f;

  streambuf * buf;
  ofstream of;
  if(_output_file_path.compare("")) {
      of.open(_output_file_path.c_str());
      buf = of.rdbuf();
  } else {
      buf = std::cout.rdbuf();
  }
  std::ostream out(buf);

  out << "Wear leveling statistics" << endl;
  out << "========================" << endl;
  out << "Total number of blocks : " << f.getStructuralParameter(SP_BLOCK_NUM) << endl;
  out << "Total number of erase operations is : " << getTotalNumberOfEraseOperations(f) << endl;
  out << "  - Legacy : " << getTotalNumberOfEraseOperations(f, true, false) << endl;
  out << "  - Multi-plane : " << getTotalNumberOfEraseOperations(f, false, true) << endl;

  out << "<---------------- cut here ---------------------->" << endl;
  out << "# Flash erase operation spreading :" << endl;
  out << "# <block index> ; <num. of erase operations on that block" << endl;

  uint32_t block_index = 0;
  for(int channel=0; channel<(int)CHANNEL_NUM; channel++)
      for(int lun=0; lun<(int)LUN_NUM; lun++)
        for(int plane=0; plane<(int)PLANE_NUM; plane++)
	  for(int block=0; block<(int)BLOCK_NUM; block++)
	  {
	    uint64_t erase_num = 0;
	    erase_num += f.getBlockStat(Address(channel, lun, plane, block, 0), LEGACY_ERASE_NUM);
	    erase_num += f.getBlockStat(Address(channel, lun, plane, block, 0), MULTIPLANE_ERASE_BLOCK);
	    out << block_index << ";" << erase_num << endl;
	    block_index++;
	  }

  if(of.is_open())
    of.close();
  return 0;
}

uint32_t WearLevelingOutput::getTotalNumberOfEraseOperations
  (FlashSystem& f, bool count_legacy, bool count_multi_plane)
{
  uint64_t res = 0;

  for(int channel=0; channel<(int)CHANNEL_NUM; channel++)
    for(int lun=0; lun<(int)LUN_NUM; lun++)
      for(int plane=0; plane<(int)PLANE_NUM; plane++)
	for(int block=0; block<(int)BLOCK_NUM; block++)
	{
	  if(count_legacy)
	    res += f.getBlockStat(Address(channel, lun, plane, block, 0), LEGACY_ERASE_NUM);
	  if(count_multi_plane)
	    res += f.getBlockStat(Address(channel, lun, plane, block, 0), MULTIPLANE_ERASE_BLOCK);
	}

  return res;
}
