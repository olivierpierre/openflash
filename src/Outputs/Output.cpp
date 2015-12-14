#include "Output.hpp"

#include "../Globs/Common.hpp"
#include "../Globs/Simulation.hpp"
#include "Outputs/WearLeveling.hpp"
#include "Outputs/Jffs2Output.hpp"
#include "Outputs/VfsOutput.hpp"
#include "Outputs/MtdOutput.hpp"

OutputPool *OutputPool::_singleton = NULL;

Output::Output (string output_file_path, Simulation &simu) :
    _output_file_path(output_file_path), _simu(simu) {}
Output::Output (Simulation &simu) : _output_file_path(""), _simu(simu) {}
Output::~Output() {}

OutputPool* OutputPool::getInstance ()
{
  if(_singleton == NULL)
    _singleton = new OutputPool();

  return _singleton;
}

void OutputPool::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

int OutputPool::output ()
{
  for(int i=0; i<(int)_outputs.size(); i++)
  {
    if(_outputs[i]->output())
      ERROR("Output generation failed ...");
#ifdef VERBOSE
    cout << "Processed output : " << _outputs[i]->getName() << endl;
#endif /* VERBOSE */
  }

  return 0;
}

OutputPool::OutputPool ()
{
  /* wear leveling */
  if(Param::getInstance()->isParamPresent("outputs.wear_leveling"))
  {
    if(!Param::getInstance()->getString("outputs.wear_leveling").compare("STDOUT"))
      _outputs.push_back(new WearLevelingOutput(*Simulation::getInstance()));
    else
      _outputs.push_back(new WearLevelingOutput(
	  Param::getInstance()->getString("outputs.wear_leveling"), *Simulation::getInstance()));
  }

  /* Jffs2 */
  if(Param::getInstance()->isParamPresent("outputs.jffs2"))
  {
    Jffs2 *jffs2 = static_cast<Jffs2 *>(Simulation::getInstance()->_flash_file_system);
    if(!Param::getInstance()->getString("outputs.jffs2").compare("STDOUT"))
      _outputs.push_back(new Jffs2Output(*Simulation::getInstance(),
	     jffs2->getStats()));
    else
      _outputs.push_back(new Jffs2Output(
	  Param::getInstance()->getString("outputs.jffs2"), *Simulation::getInstance(),
	  jffs2->getStats()));
  }

  /* VFS */
  if(Param::getInstance()->isParamPresent("outputs.vfs"))
  {
    VirtualFileSystem *vfs = static_cast<VirtualFileSystem *>(Simulation::getInstance()->_virtual_file_system);
    if(!Param::getInstance()->getString("outputs.vfs").compare("STDOUT"))
      _outputs.push_back(new VfsOutput(*Simulation::getInstance(),
	     vfs->getStats()));
    else
      _outputs.push_back(new VfsOutput(
	  Param::getInstance()->getString("outputs.vfs"), *Simulation::getInstance(),
	  vfs->getStats()));
  }

  /* Mtd */
  if(Param::getInstance()->isParamPresent("outputs.mtd"))
  {
    NandDriver *mtd = Simulation::getInstance()->_nand_driver;
    if(!Param::getInstance()->getString("outputs.mtd").compare("STDOUT"))
      _outputs.push_back(new MtdOutput(*Simulation::getInstance(),
	     mtd->getStats()));
    else
      _outputs.push_back(new MtdOutput(
	  Param::getInstance()->getString("outputs.mtd"), *Simulation::getInstance(),
	  mtd->getStats()));
  }


#ifdef VERBOSE
  cout << "[OUTPUTS] Outputs for this simulation : ";
  for(int i=0; i<(int)_outputs.size(); i++)
    cout << _outputs[i]->getName() << ", ";
  cout << endl;
#endif /* VERBOSE */

}

OutputPool::~OutputPool ()
{
  for(int i=0; i<(int)_outputs.size(); i++)
    delete(_outputs[i]);
}
