/*
 * Jffs2WriteBuffer.cpp
 *
 *  Created on: 17 mars 2014
 *      Author: pierre
 */

#include "Jffs2WriteBuffer.hpp"
#include "../VFS/VFS.hpp"

Jffs2WriteBuffer::Jffs2WriteBuffer (Jffs2 *fs, int size)
{
  _fs = fs;
  _size = size;
  _d = _fs->_d;


  _wbuf_offset = 0;
  _current_page = 0xFFFFFFFF;
  _current_block = NULL;
}

/**
 * Send a node to the write buffer
 */
PpcValF Jffs2WriteBuffer::writeNode(Jffs2Node *n, mtd_call_src_t src)
{
  PpcValF res = {0, 0};
  int to_write = n->getFlashSize();

  /* set the flash_offset member for the node */
  n->_flash_offset = ((_current_block->getIndex()*PAGES_PER_BLOCK+_current_page) * PAGE_SIZE_BYTE) + _wbuf_offset;

  /* A node has a maximum size */
  assert(to_write <= (int)(LINUX_PAGE_SIZE+max(SIZEOF_JFFS2_RAW_INODE, SIZEOF_JFFS2_RAW_DIRENT)));

  while(to_write != 0)
  {
    //STEP 1 - write as far as possible in the buffer / and the end of the node
    if(to_write > 0)
    {
      int written;
      res = res + fillBuffer(n, to_write, &written, src);
      to_write -= written;
    }

    //STEP 2 - write full pages
    while(to_write >= _size)
    {
      // flush if needed
      flush(src);

      // direct write for the full pages
      vector<Jffs2Node *>v; v.push_back(n);
      int nb_full_pages_to_write = to_write / PAGE_SIZE_BYTE;
      for(int i=0; i<nb_full_pages_to_write; i++)
      {
	res = res + writePageAndUpdateIfNeeded(v, src);
	to_write -= _size;
      }
    }

    assert(to_write >= 0);
  }

  return res;
}

/**
 * Flush the write buffer if needed
 *
 */
PpcValF Jffs2WriteBuffer::flush(mtd_call_src_t src)
{
  PpcValF res = {0, 0};
  if(empty())				// flush not needed
    return res;

  //do the actual write
  res = res + writePageAndUpdateIfNeeded(_content, src);

  // reset buffer
  _wbuf_offset = 0;
  _content.clear();

  return res;
}

/**
 * Stack a new node (or part of a node) in the write buffer
 * The size of the node / part of the node is indicated in size
 * May trigger a flush if we arrive at the end of the buffer
 * The number of bytes actually written in the buffer is set in *written
 */
PpcValF Jffs2WriteBuffer::fillBuffer(Jffs2Node *n, int size, int *written, mtd_call_src_t src)
{
  PpcValF res = {0, 0};
  *written = 0;
  assert(_wbuf_offset < _size);

  _content.push_back(n);
  *written = min(_size - _wbuf_offset, size);
  _wbuf_offset = PAD(_wbuf_offset + *written);

  assert(_wbuf_offset <= _size);

  if(_wbuf_offset == _size)
    res = res + flush(src);

  return res;
}

/**
 * Is the wbuf empty ?
 */
bool Jffs2WriteBuffer::empty ()
{
  return _content.empty();
}

/**
 * Write @ current block & page, update current block & current page if needed
 * updated is set to true if the current block is updated
 */
PpcValF Jffs2WriteBuffer::writePageAndUpdateIfNeeded(vector<Jffs2Node *> &content, mtd_call_src_t src)
{
  PpcValF res = {0, 0};

  /* 1. Compute address */
  Address addr = _d->byteAddrToPage((_current_block->getIndex()*PAGES_PER_BLOCK + _current_page)*PAGE_SIZE_BYTE);

  /* 2 . Prepare priv data */
  vector<Jffs2Node *> *priv = new vector<Jffs2Node *>();
  for(int i=0; i<(int)content.size(); i++)
    priv->push_back(content[i]);

  /* 3. write page */
  res = res + _d->writePage(addr, src);

  /* 4. set page priv */
  _fs->_f->setPagePriv(addr, priv);

  /* 5. update block metadata */
  for(int i=0; i<(int)priv->size(); i++)
    _current_block->addNode(priv->at(i));

#ifdef VERBOSE
  cout << "[JFFS2:" << TIME_NOW << "] Written B" << _current_block->getIndex()
      << "|" << "P" << _current_page << " with : ";
  for(int i=0; i<(int)content.size(); i++)
  {
      Jffs2Node &n = *(content[i]);
    if(n.getType() == JFFS2_DATA)
      cout << "Da";
    else if(n.getType() == JFFS2_DIRENT)
      cout << "Di";
    else
      ERROR("Bad jffs2 node type");
    cout << n.getInodeNum() << "-v" << n.getVersion() << "-@[" << n.getFlashOffset() <<
	"->" << n.getFlashOffset()+n.getFlashSize()-1 << "], ";
  }
  cout << endl;
#endif /* VERBOSE */

  if(++_current_page == (uint32_t)PAGES_PER_BLOCK)
    updateCurrentBlock();

  return res;
}

/**
 * Return the current page present in the write buffer
 */
Address Jffs2WriteBuffer::getAddress()
{
  Address res = _d->byteAddrToPage(_current_block->getIndex()*BLOCK_SIZE_BYTE + _current_page*PAGE_SIZE_BYTE);
  return res;
}

/**
 * Returns in bytes the amount of free space remaining in
 * the current block
 */
uint32_t Jffs2WriteBuffer::getFreeSpaceLeftInCurrentBlock ()
{
  uint32_t used_space = _current_page*PAGE_SIZE_BYTE + _wbuf_offset;

  return BLOCK_SIZE_BYTE - used_space;
}

/**
 * Force the selection of a new block for _current_block
 */
PpcValF Jffs2WriteBuffer::forceNewBlock (mtd_call_src_t src)
{
  PpcValF res = {0, 0};

  /* ok here the flush function may itself update the current block
   * so we have to keep watch if it is the case, we do not re-update the
   * current block !
   */
  Jffs2Block * old_cb = _current_block;
  res = res + flush(src);
  if(old_cb == _current_block)
    updateCurrentBlock();

  return res;
}

int Jffs2WriteBuffer::updateCurrentBlock()
{
  if(_current_block != NULL)
    _fs->_gc->classifyOldCurrentBlock(_current_block);

  _current_block = _fs->getFreeBlock();
  _current_page = 0;
  return 0;
}

uint32_t Jffs2WriteBuffer::getCurrentBLockIndex ()
{
  return _current_block->getIndex();
}
