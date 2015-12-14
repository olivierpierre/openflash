/*
 * Jffs2WriteBuffer.hpp
 *
 *  Created on: 17 mars 2014
 *      Author: pierre
 */

#ifndef JFFS2WRITEBUFFER_HPP
#define JFFS2WRITEBUFFER_HPP

#include "../../FunctionalGlobals.hpp"
#include "Jffs2File.hpp"
#include "Jffs2.hpp"
#include "Jffs2GC.hpp"

/* Written flash offsets are always 4 bytes aligned */
#define PAD(x) (((x)+3)&~3)

class Jffs2WriteBuffer
{
public:
  Jffs2WriteBuffer (Jffs2 *fs, int size);
  PpcValF writeNode(Jffs2Node *dn);
  PpcValF flush();
  bool empty();
  Address getAddress();
  uint32_t getFreeSpaceLeftInCurrentBlock();
  PpcValF forceNewBlock();
  uint32_t getCurrentBLockIndex();

private:
  int _size;			/* total size in byte, it is the size of an underlying flash page */
  int _wbuf_offset;		/* offset inside the buffer for the next write */
  Jffs2Block *_current_block;	/* current block being written */
  uint32_t _current_page;	/* index of the current page inside the current block */
  vector<Jffs2Node *> _content;	/* data & dirent nodes included in the write buffer */
  Jffs2 *_fs;			/* pointer to the file system */
  NandDriver *_d;		/* nand driver for flash r/w/e operations */

  PpcValF fillBuffer(Jffs2Node *n, int size, int *res);
  PpcValF writePageAndUpdateIfNeeded(vector<Jffs2Node *> &content);
  int updateCurrentBlock();

  friend class Jffs2;
  friend class Jffs2GC;	/* for sanity test only, remove here when we are done */
};

#endif /* JFFS2WRITEBUFFER_HPP */
