#include <string>

#include "FlashFileSystem.hpp"
#include "Jffs2/Jffs2.hpp"

using namespace std;

FlashFileSystem *FlashFileSystem::_singleton = NULL;

FlashFileSystem* FlashFileSystem::getInstance ()
{
  if(_singleton == NULL)
  {
    string mode = Param::getInstance()->getString("functional_model.functional_mode");
    if(mode.compare("ffs"))
      ERROR("Only FFS mode implemented yet, please select \"ffs\" as functional mode");

    string ffs_model = Param::getInstance()->getString("functional_model.ffs.type");
    if(ffs_model.compare("jffs2") && ffs_model.compare("JFFS2"))
      ERROR("Only JFFS2 implemented yet, please select \"JFFS2\" as FFS type");

    Jffs2 *jffs2_ptr = new Jffs2();
    _singleton = static_cast<FlashFileSystem *>(jffs2_ptr);
  }

  return _singleton;
}

void FlashFileSystem::kill ()
{
  if (_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

FlashFileSystem::FlashFileSystem(FlashSystem *f, NandDriver *d, PageCache *pc) : _f(f), _d(d), _page_cache(pc) {}
FlashFileSystem::~FlashFileSystem() {}
