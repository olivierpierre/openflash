#ifndef FLASH_FILE_SYSTEM_HPP
#define FLASH_FILE_SYSTEM_HPP

#include "../../FlashLayer/FlashLayer.hpp"
#include "../FunctionalGlobals.hpp"
#include "NandDriver.hpp"
#include "VFS/PageCache.hpp"

#define LINUX_PAGE_SIZE		4096

typedef enum
{
  JFFS2,
  YAFFS2,
  UBIFS
} flash_file_system_t;

class FlashFileSystem
{
  public:
    static FlashFileSystem * getInstance();
    static void kill();

    virtual ~FlashFileSystem()=0;
    virtual flash_file_system_t getType()=0;	//FIXME maybe we can delete this
    virtual PpcValF ffsReadPage(uint32_t inode, uint32_t page_index, int* ret)=0;
    virtual PpcValF ffsWriteBegin(uint32_t inode, uint32_t page_index, uint32_t offset, uint32_t count)=0;
    virtual PpcValF ffsWriteEnd(uint32_t inode, uint32_t page_index, uint32_t offset, uint32_t count)=0;
    virtual PpcValF ffsRemove(uint32_t inode)=0;
    virtual PpcValF ffsOpen(uint32_t inode, bool truncate, bool create, int name_len)=0;
    virtual PpcValF ffsCreate(uint32_t inode, int name_len)=0;
    virtual PpcValF ffsTruncate(uint32_t inode, uint32_t length)=0;
    virtual PpcValF ffsRename(uint32_t inode, int new_name_len)=0;
    virtual PpcValF ffsSync()=0;
    virtual PpcValF ffsClose(uint32_t inode)=0;
    //TODO GC related functions
    
  protected:
    FlashFileSystem(FlashSystem *f, NandDriver *d, PageCache *pc);
    FlashSystem *_f;
    NandDriver *_d;
    PageCache *_page_cache;			/* page cache */

  private:
    static FlashFileSystem *_singleton;
};

#include "Jffs2/Jffs2.hpp"

#endif /* FLASH_FILE_SYSTEM_HPP */
