/*
 * Jffs2GC.hpp
 *
 *  Created on: 4 avr. 2014
 *      Author: pierre
 */

#ifndef JFFS2GC_HPP
#define JFFS2GC_HPP

#include "Jffs2.hpp"

#define	JFFS2GC_MIN_DIRTY_SIZE		((uint32_t)256)
#define JFFS2GC_MIN_VERY_DIRTY_SIZE	((uint32_t)BLOCK_SIZE_BYTE/(uint32_t)2)

#define JFFS2_IS_VERY_DIRTY(size)		(size >= JFFS2GC_MIN_VERY_DIRTY_SIZE)
#define JFFS2_IS_DIRTY(size)			(size >= JFFS2GC_MIN_DIRTY_SIZE)

class Jffs2GC;

/**
 * Jffs2 internal representation of a flash block
 * A block can be in only one of those different places :
 * - (jffs2->_wbuf->_current_block) --> currently being written
 * - Jffs2GC->current_gc_block --> currently being gc
 * - Free list --> completely free, ready to be written
 * - Clean list --> full of valid nodes (or less than 256 bytes of obsolete data)
 * - Dirty list --> more than 256 bytes but less than (BLK_SIZE_BYTE/2) bytes of obsolete data
 * - Very Dirty list -->more than (BLK_SIZE_BYTE/2)bytes of obsolete data
 * - erase pending, erase complete
 * - erasable list
 */
class Jffs2Block
{
public:
  Jffs2Block(uint64_t offset);
  uint32_t getIndex() {return _offset/BLOCK_SIZE_BYTE;};
  uint32_t getOffset() {return _offset;};
  uint32_t getUsedSize();
  uint32_t getDirtySize();
  uint32_t getFreeSize();
  uint32_t getNodesNum() {return _nodes.size();};
  Jffs2Node *getNodeAtIndex(int index) {return _nodes[index];};
  int addNode(Jffs2Node *n);
  void clearNodeList();
  bool isCompletelyInvalid();

private:
  uint64_t _offset;
  vector<Jffs2Node *> _nodes;

  /* for debug purpose only remove when we are done */
  friend class Jffs2GC;
};

class Jffs2GC
{
public:
  Jffs2GC (Jffs2 *fs);
  ~Jffs2GC ();

  PpcValF garbageCollectorPass();
  Jffs2Block *getFreeBlock();
  int classifyOldCurrentBlock(Jffs2Block *blk);
  int reclassifyBlockAtFlashOffset(uint64_t flash_offset);
  Jffs2Block *getBlockToGC();
  PpcValF eraseFromErasePending();
  PpcValF checkAndMarkCleanFromEraseComplete();
  PpcValF moveOneNode();
  bool gcdShouldWake();
  int gcdTrigger();
  void gcdDone();

  /* stats */
  uint32_t stats_getFreeSpace() const;

  /* debug */
  int blocklistSanityCheck();
  void printBlockLists();

private:
  Jffs2 *_fs;
  Jffs2Block * _current_gc_block;
  uint32_t _next_node_to_gc;

  int _very_dirty_blocks_gc_trigger;
  int _resv_blocks_gc_trigger;
  uint64_t _no_spc_dirty_size;

  vector<Jffs2Block *> _blocks;		/* all blocks */

  vector<Jffs2Block *> _free_list;
  vector<Jffs2Block *> _dirty_list;
  vector<Jffs2Block *> _very_dirty_list;
  vector<Jffs2Block *> _clean_list;
  vector<Jffs2Block *> _eraseable_list;
  vector<Jffs2Block *> _erase_pending;
  vector<Jffs2Block *> _erase_complete;

  bool _gc_in_progress;
  bool _gcd_enabled;			/* gc thread */
  bool _gcd_scheduled;
};

#endif /* JFFS2GC_HPP */
