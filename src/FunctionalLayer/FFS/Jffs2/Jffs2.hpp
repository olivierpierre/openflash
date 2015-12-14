#ifndef JFFS2_HPP
#define JFFS2_HPP

#include "../FlashFileSystem.hpp"
#include "Jffs2File.hpp"
#include "Jffs2Stats.hpp"

class Jffs2WriteBuffer;
class Jffs2GC;
class Jffs2Block;
class VirtualFileSystem;
class OutputPool;

class Jffs2 : public FlashFileSystem
{
  public:
    Jffs2();
    ~Jffs2();
    flash_file_system_t getType();

    PpcValF ffsReadPage (uint32_t inode, uint32_t page_index, int *ret);
    PpcValF ffsWriteBegin(uint32_t inode_num, uint32_t page_index, uint32_t offset, uint32_t count);
    PpcValF ffsWriteEnd(uint32_t inode_num, uint32_t page_index, uint32_t offset, uint32_t count);
    PpcValF ffsRemove(uint32_t inode_num);
    PpcValF ffsOpen(uint32_t inode_num, bool truncate, bool create, int name_len);
    PpcValF ffsCreate(uint32_t inode_num, int name_len);
    PpcValF ffsTruncate(uint32_t inode, uint32_t length);
    PpcValF ffsRename(uint32_t inode, int new_name_len);
    PpcValF ffsClose(uint32_t inode);
    PpcValF ffsSync();

    Jffs2Stats *getStats() {return _stats;};

    /* debug : */
    void printFileSystem();

  private:
    vector<Jffs2File *> _files;			/* set of all files in the filesystem */

    Jffs2WriteBuffer *_wbuf;			/* write buffer */
    Jffs2GC *_gc;				/* garbage collector (block lists there) */
    Jffs2Stats *_stats;

    double _base_readpage_timing_overhead;
    double _base_write_end_timing_overhead;
    double _base_write_begin_timing_overhead;

    bool _read_check;

    PpcValF reserveSpace(uint32_t bytes_needed);
    Jffs2Block *getFreeBlock();
    Jffs2File *getFile(uint32_t inode_num, bool must_exist=true);
    int delFile(Jffs2File *f);

    friend class Jffs2WriteBuffer;
    friend class Jffs2GC;
    friend class OuputPool;
    friend class Jffs2GcThread;
};

#include "Jffs2WriteBuffer.hpp"
#include "Jffs2GC.hpp"

#endif /* JFFS2_HPP */
