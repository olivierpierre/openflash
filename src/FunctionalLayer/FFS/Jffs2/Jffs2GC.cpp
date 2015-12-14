/*
 * Jffs2GC.cpp
 *
 *  Created on: 4 avr. 2014
 *      Author: pierre
 */

#include "Jffs2GC.hpp"
#include "../../../TimeEvents/CustomAsyncEvents/FunctionalEvents/Jffs2GcThread.hpp"

#include <vector>
#include <algorithm>

using namespace std;

int index_in_list(Jffs2Block *blk, vector<Jffs2Block *> list);

/******************************* JFFS2BLOCK ************************************/


Jffs2Block::Jffs2Block (uint64_t offset) : _offset(offset) {}

/**
 * Return the number of used bytes in the block
 */
uint32_t Jffs2Block::getUsedSize ()
{
  uint32_t res = 0;

  for(int i=0; i<(int)_nodes.size(); i++)
    res += _nodes[i]->getFlashSize();

  return res;
}

/**
 * Return the number of obsolete bytes in the block
 * Note : should not be called on the block currently gc'ed
 */
uint32_t Jffs2Block::getDirtySize ()
{
  uint32_t res = 0;

  for(int i=0; i<(int)_nodes.size(); i++)
    if(_nodes[i]->getState() == JFFS2_NODE_INVALID)
      res += _nodes[i]->getFlashSize();

  return res;
}

/**
 * Return the number of free bytes in the block
 * Note : should not be called on the block currently gc'ed
 */
uint32_t Jffs2Block::getFreeSize ()
{
  return BLOCK_SIZE_BYTE-getUsedSize();
}

/**
 * Add a node to the nodelist of the block. We must check if it is not equal to the last node
 * inserted. Indeed, as addNode is called from the write buffer when the flash write actually
 * happen, a node overlapping several flash pages will trigger several calls to addNode related
 * to the same node. Add it just one time.
 */
int Jffs2Block::addNode (Jffs2Node* n)
{
  if(_nodes.empty() || _nodes.back() != n)
  _nodes.push_back(n);
  return 0;
}

/**
 * Empty the node list of the block
 */
void Jffs2Block::clearNodeList ()
{
  _nodes.clear();
}

/**
 * Return true if the block contains no valid node
 */
bool Jffs2Block::isCompletelyInvalid ()
{
  for(int i=0; i<(int)_nodes.size(); i++)
    if(_nodes[i]->getState() == JFFS2_NODE_VALID)
      return false;

  return true;
}

/******************************** JFFS2GC *************************************/

Jffs2GC::Jffs2GC (Jffs2 *fs)
{
  _fs = fs;
  _current_gc_block = NULL;
  _next_node_to_gc = 0;

  /* init all blocks */
  for(int block_index = 0; block_index <(int)BLOCK_NUM; block_index++)
    _blocks.push_back(new Jffs2Block(block_index*BLOCK_SIZE_BYTE));

  /* put them all in the free list first */
  for(int i=0; i<(int)_blocks.size(); i++)
    _free_list.push_back(_blocks[i]);

  /* compute _very_dirty_blocks_gc_trigger & friends */
  uint64_t size = ((BLOCK_NUM*BLOCK_SIZE_BYTE)/50) + (BLOCK_NUM * 100) +  BLOCK_SIZE_BYTE - 1;
  _very_dirty_blocks_gc_trigger = _resv_blocks_gc_trigger = 2 + (size / BLOCK_SIZE_BYTE) + 1;
  _no_spc_dirty_size = BLOCK_SIZE_BYTE + (((BLOCK_NUM*BLOCK_SIZE_BYTE)/50) / 100);

  /* GC thread */
  _gcd_enabled = Param::getInstance()->getBool("functional_model.ffs.jffs2.bg_thread");

  _gc_in_progress = false;
  _gcd_scheduled = false;
}

PpcValF Jffs2GC::garbageCollectorPass ()
{
  PpcValF res = {0, 0};

  assert(!_gc_in_progress);
  _gc_in_progress = true;

#ifdef VERBOSE
  cout << "[JFFS2] Gabage collector pass" << endl;
#endif /* VERBOSE */

  /* if we can set a clean block now ... */
  if(!_erase_complete.empty() || !_erase_pending.empty())
  {
    if(_erase_complete.empty())
      res = res + eraseFromErasePending();
    res = res + checkAndMarkCleanFromEraseComplete();
    goto out;
//    _gc_in_progress = false;
//    return res;
  }

  /* if no block selected to gc get one */
  if(_current_gc_block == NULL)
  {
    _current_gc_block = getBlockToGC();
    _next_node_to_gc = 0;
  }

  /* cant find any block to gc ... */
  if(_current_gc_block == NULL)
  {
#ifdef VERBOSE
    cout << "Could not find any block to gc, bye." << endl;
#endif /* VERBOSE */
    goto out;
//    _gc_in_progress = false;
//    return res;

  }

  /* is the block being currently gc'ed ready to be erased ? */
//  if(_current_gc_block->getDirtySize() == (uint32_t)BLOCK_SIZE_BYTE)
//  {
//    _erase_pending.push_back(_current_gc_block);
//    _current_gc_block = NULL;
//    return res;
//  }

  /* we have to move one node. After that if the block is
   * completely dirty this function will put it in erase pending
   */
  res = res + moveOneNode();

#ifndef NDEBUG
  blocklistSanityCheck();
#endif

out:
  _gc_in_progress = false;
  _fs->_stats->addJffs2CallTimingInfo(JFFS2_GC_PASS, res.time, res.e_cpu, res.e_mem);
  return res;
}

Jffs2GC::~Jffs2GC ()
{
  for(int i=0; i<(int)_blocks.size(); i++)
    delete _blocks[i];
}

/**
 * Pop a block from the free list
 */
Jffs2Block* Jffs2GC::getFreeBlock ()
{
  if(_free_list.empty())
  {
    printBlockLists();
    cout << _blocks[0]->_nodes.size() << endl;
  }
  /* TODO implement what to do when
   * that list is empty
   */
  assert(!_free_list.empty());

  Jffs2Block *block = _free_list.back();
    _free_list.pop_back();

#ifdef VERBOSE
  uint32_t index = block->getIndex(); (void)index;
  cout << "[JFFS2] Picked new free block " << block->getOffset() / BLOCK_SIZE_BYTE << " from free list" << endl;
#endif /* VERBOSE */

  return block;
}

/**
 * Given a block which was the last currently block and has be fully written,
 * put that block in the clean, dirty or very_dirty list according to its content
 */
int Jffs2GC::classifyOldCurrentBlock (Jffs2Block* blk)
{

#ifndef NDEBUG
  // sanity test, the block must not be in another list
  if(index_in_list(blk, _free_list) != -1 ||
      index_in_list(blk, _clean_list) != -1 ||
      index_in_list(blk, _dirty_list) != -1 ||
      index_in_list(blk, _very_dirty_list) != -1 ||
      index_in_list(blk, _eraseable_list) != -1 ||
      index_in_list(blk, _erase_pending) != -1 ||
      index_in_list(blk, _erase_complete) != -1)
	assert(0);

  // another sanity test check that the block is sufficiently filled
  assert(blk->_nodes.size() > 10);
#endif /* NDEBUG */

  uint32_t dirty_size = blk->getDirtySize();
  if(JFFS2_IS_DIRTY(dirty_size))
  {
    if(JFFS2_IS_VERY_DIRTY(dirty_size))
    {
      _very_dirty_list.push_back(blk);
#ifdef VERBOSE
      cout << "[JFFS2] putting old current block " << blk->getIndex() << " in very dirty list" << endl;
#endif /* VERBOSE */
    }
    else
    {
      _dirty_list.push_back(blk);
#ifdef VERBOSE
      cout << "[JFFS2] putting old current block " << blk->getIndex() << " in dirty list" << endl;
#endif /* VERBOSE */
    }
  }
  else
  {
    _clean_list.push_back(blk);
#ifdef VERBOSE
      cout << "[JFFS2] putting old current block " << blk->getIndex() << " in clean list" << endl;
#endif /* VERBOSE */
  }

  return 0;
}

/**
 * Called by a node when it is invalidated to reclassify the block if needed.
 * it is a node offset so we know the total node will not cross block boundaries
 */
int Jffs2GC::reclassifyBlockAtFlashOffset (uint64_t flash_offset)
{
  Jffs2Block *block = _blocks[flash_offset / BLOCK_SIZE_BYTE];

  /* the offset is in the current block, do nothing, the current block will be classified
   * when it is full */
  if(block == _fs->_wbuf->_current_block || block == _current_gc_block)
    return 0;

  // sanity test, the block must not be in free / eraseable / pending / complete
#ifndef NDEBUG
  if(index_in_list(block, _free_list) != -1 ||
      index_in_list(block, _eraseable_list) != -1 ||
      index_in_list(block, _erase_complete) != -1 ||
      index_in_list(block, _erase_pending) != -1)
	assert(0);
#endif /* NDEBUG */

  /* So the block can be in clean, dirty, very dirty */
  int block_index_in_list;
  block_index_in_list = index_in_list(block, _clean_list);
  if(block_index_in_list != -1)
  {
    if(JFFS2_IS_DIRTY(block->getDirtySize()))
    {
      /* a block in clean list has became dirty */
#ifdef VERBOSE
      cout << "[JFFS2] GC : re-classify previously clean block "
	  << block->getIndex() << " in dirty list" << endl;
#endif /* VERBOSE */
      _clean_list.erase(_clean_list.begin() + block_index_in_list);
      _dirty_list.push_back(block);
    }
  }
  else if ((block_index_in_list = index_in_list(block, _dirty_list)) != -1)
  {
    /* it may have became very dirty */
    if(JFFS2_IS_VERY_DIRTY(block->getDirtySize()))
    {
#ifdef VERBOSE
      cout << "[JFFS2] GC : re-classify previously dirty block "
	  << block->getIndex() << " in very dirty list" << endl;
#endif /* VERBOSE */
      _dirty_list.erase(_dirty_list.begin() + block_index_in_list);
      _very_dirty_list.push_back(block);
    }
    else if(block->isCompletelyInvalid())	/* or completely invalid ? unlikely */
    {
#ifdef VERBOSE
      cout << "[JFFS2] GC : re-classify previously dirty block "
	  << block->getIndex() << " as completely invalid" << endl;
#endif /* VERBOSE */
      _dirty_list.erase(_dirty_list.begin() + block_index_in_list);
      if(rand()%128 != 0)
	_erase_pending.push_back(block);
      else
	_eraseable_list.push_back(block);
    }

  }
  else
  {
    block_index_in_list = index_in_list(block, _very_dirty_list);
    assert(block_index_in_list != -1);

    /* it may have became completely invalid */
    if(block->isCompletelyInvalid())
    {
#ifdef VERBOSE
      cout << "[JFFS2] GC : re-classify previously very dirty block "
	  << block->getIndex() << " as completely invalid" << endl;
#endif /* VERBOSE */
      _very_dirty_list.erase(_very_dirty_list.begin() + block_index_in_list);
      if(rand()%128 != 0)
	_erase_pending.push_back(block);
      else
	_eraseable_list.push_back(block);
    }
  }

  return 0;
}

/**
 * Choose the next victim block for garbage collection
 * Return NULL if a block cannot be found (for example when the only non free block
 * is the block currently being written)
 * Also remove the selected block from the list where it belongs
 */
Jffs2Block* Jffs2GC::getBlockToGC ()
{
  Jffs2Block *res = NULL;
  int n = rand()%128;
  vector<Jffs2Block *> *selected_list = &_clean_list;

  assert(_current_gc_block == NULL);

  if(n < 50 && !_eraseable_list.empty())
  {
    selected_list = &_eraseable_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from erasable list" << endl;
#endif /* VERBOSE */
  }
  else if(n < 110 && !_very_dirty_list.empty())
  {
    selected_list = &_very_dirty_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from very dirty list" << endl;
#endif /* VERBOSE */
  }
  else if(n < 126 && !_dirty_list.empty())
  {
    selected_list = &_dirty_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from dirty list" << endl;
#endif /* VERBOSE */
  }
  else if(!_clean_list.empty())
  {
    selected_list = &_clean_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from clean list (!)" << endl;
#endif /* VERBOSE */
  }
  else if(!_dirty_list.empty())
  {
    selected_list = &_dirty_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from dirty list (!)" << endl;
#endif /* VERBOSE */
  }
  else if(!_very_dirty_list.empty())
  {
    selected_list = &_very_dirty_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from very dirty list (!)" << endl;
#endif /* VERBOSE */
  }
  else if(!_eraseable_list.empty())
  {
    selected_list = &_eraseable_list;
#ifdef VERBOSE
    cout << "[JFFS2] picking new block to gc from erasable list (!)" << endl;
#endif /* VERBOSE */
  }						// TODO here there is one more case with the
  else						// wbuf pending
  {
    return NULL;
#ifdef VERBOSE
    cout << "[JFFS2] could not find any block to gc" << endl;
#endif /* VERBOSE */
  }

  res = selected_list->front();
  selected_list->erase(selected_list->begin());

#ifdef VERBOSE
  cout << "[JFFS2] block chosen :" << res->getIndex() << endl;
#endif /* VERBOSE */

  assert(res->getNodesNum() > 0);

  return res;
}

/**
 * Erase a block from the erase pending list, and move it to the erase complete list
 */
PpcValF Jffs2GC::eraseFromErasePending ()
{
  PpcValF res = {0, 0};
  assert(!_erase_pending.empty());

  Jffs2Block *b = _erase_pending.front();
  _erase_pending.erase(_erase_pending.begin());

  //clear its nodelist
  b->clearNodeList();

  // clear priv data for all pages
  Address addr_priv = NandDriver::getInstance()->byteAddrToPage(b->getOffset());
  for(int i=0; i<(int)PAGES_PER_BLOCK; i++)
  {
    addr_priv.setPage(i);
    vector<Jffs2Node *> *priv_ptr = (vector<Jffs2Node *> *)_fs->_f->getPagePriv(addr_priv);
    delete priv_ptr;
    _fs->_f->setPagePriv(addr_priv, NULL);
  }

  // erase the block
  Address addr = NandDriver::getInstance()->byteAddrToPage(b->getOffset());
  res = res + _fs->_d->eraseBlock(addr);
//  LegacyErase le(addr);
//  res.time += le.getTimeTaken();
//  res.power_consumption += le.getEnergyConsumed();

  // move to erase complete
  _erase_complete.push_back(b);

#ifdef VERBOSE
      cout << "[JFFS2] Erased 1 block (" << b->getIndex() << ") from erase pending --> erase_complete" << endl;
#endif /* VERBOSE */

  return res;
}

/**
 * Check if necessary and write a clean marker for one block of the erase complete
 * list, then move it to the clean list
 */
PpcValF Jffs2GC::checkAndMarkCleanFromEraseComplete ()
{
  PpcValF res = {0, 0};
  assert(!_erase_complete.empty());

  Jffs2Block *b = _erase_complete.front();
  _erase_complete.erase(_erase_complete.begin());

  /* check after erase ? */
  if(Param::getInstance()->getBool("functional_model.ffs.jffs2.check_after_erase"))
  {
    Address addr = NandDriver::getInstance()->byteAddrToPage(b->getOffset());
    for(int i=0; i<(int)PAGES_PER_BLOCK; i++)
    {
      addr.setPage(i);
      res = res + _fs->_d->readPage(addr);
//      LegacyRead lr(addr);
//      res.time += lr.getTimeTaken();
//      res.power_consumption += lr.getEnergyConsumed();
    }
  }

  /* TODO write cleanmarker ? do we simulate that ?? */

  // put the block in the free list
  _free_list.insert(_free_list.begin(), b);

#ifdef VERBOSE
      cout << "[JFFS2] Moved 1 block (" << b->getIndex() << ") from erase complete -> free_list" << endl;
#endif /* VERBOSE */

  return res;
}

/**
 * Move a node from the current block being gc'ed. Update dirty size, etc. If the block is
 * completely dirty, put it in erase pending.
 * If the node was previously invalid we can delete it from the file node list, and free the
 * related memory. If it was not invalid, we do not delete nor free it, and we have to
 * update the flash_offset member of that node as we write it somewhere else.
 */
PpcValF Jffs2GC::moveOneNode ()
{
  PpcValF res = {0, 0};

  /* move one node */
  bool work_done = false;
  while(!work_done)
  {
    Jffs2Node *n = _current_gc_block->getNodeAtIndex(_next_node_to_gc);

    /* invalid node ? delte from fs metadata & skip */
    if(n->getState() == JFFS2_NODE_INVALID)
    {
#ifdef VERBOSE
      cout << "[JFFS2] block " << _current_gc_block->getIndex() << " node " <<
	  _next_node_to_gc << "/" << _current_gc_block->getNodesNum()-1 <<
	  " (" << *n << ") is invalid, skipping" << endl;
#endif /* VERBOSE */
      // remove the node from filesystem metadata
      Jffs2File *f = _fs->getFile(n->getInodeNum(), false);
      assert(f != NULL);
      int must_delete_file = f->deleteNode(n);

      // this was the last node for that file lets delete it
      if(must_delete_file == 1)
	_fs->delFile(f);
    }
    else if(n->getState() == JFFS2_NODE_VALID)
    {
      /* valid node ? move to another location and update offset */

      /* read the page */
      vector<Address> pages_to_read = _fs->_d->byteRangeToPages(n->getFlashOffset(), n->getFlashSize());
      for(int i=0; i<(int)pages_to_read.size(); i++)
	res = res + _fs->_d->readPage(pages_to_read[i]);	/* TODO check here ! */

#ifdef VERBOSE
      cout << "[JFFS2] block " << _current_gc_block->getIndex() << " node " <<
	  _next_node_to_gc << "/" << _current_gc_block->getNodesNum()-1 <<
	  " (" << *n << ") is valid, moving it" << endl;
#endif /* VERBOSE */

      /* we must assure there is enough space in the current block */
      if(_fs->_wbuf->getFreeSpaceLeftInCurrentBlock() < n->getFlashSize())
	res = res + _fs->_wbuf->forceNewBlock();

      /* move the node */
      res = res + _fs->_wbuf->writeNode(n);

      /* done */
      work_done = true;
    }
    else
      ERROR("Invalid jffs2 node state");


    /* next node */
    _next_node_to_gc++;

    /* end of the block ? */
    if(_next_node_to_gc == _current_gc_block->getNodesNum())
      work_done = true;
  }

  /** block fully invalid, put it in erase pending */
  if(_next_node_to_gc == _current_gc_block->getNodesNum())
  {

#ifdef VERBOSE
    cout << "[JFFS2] processed last valid node of gc'ed block " << _current_gc_block->getIndex() <<
	", putting it on erase pending" << endl;
#endif /* VERBOSE */

    _erase_pending.push_back(_current_gc_block);
    _current_gc_block = NULL;
    _next_node_to_gc = 0;

    return res;
  }


  return res;
}

/*********************** TOOLS ***************************/

/**
 * If the given block is in the given list, return the index
 * of that blokc in that list. If not, return -1
 */
int index_in_list(Jffs2Block *blk, vector<Jffs2Block *> list)
{
  for(int i=0; i<(int)list.size(); i++)
    if(blk == list[i])
      return i;
  return -1;
}

/**
 * Return the flash amount of free space in bytes
 */
uint32_t Jffs2GC::stats_getFreeSpace () const
{
  return _free_list.size()*BLOCK_SIZE_BYTE +
      _fs->_wbuf->getFreeSpaceLeftInCurrentBlock();
}

int Jffs2GC::blocklistSanityCheck ()
{
  /* each block must be in one and only one place */
  for(int i=0; i<(int)_blocks.size(); i++)
  {
    Jffs2Block *b = _blocks[i];
    int occurences = 0;

    /* 1. currently written block */
    if(_fs->_wbuf->_current_block == b)
      occurences++;

    /* 2. currently gc'ed block */
    if(_current_gc_block == b)
      occurences++;

    /* 3. free_list */
    if(!_free_list.empty())
      if(find(_free_list.begin(), _free_list.end(), b) != _free_list.end())
	occurences++;

    /* 4. clean list */
    if(!_clean_list.empty())
      if(find(_clean_list.begin(), _clean_list.end(), b) != _clean_list.end())
    	occurences++;

    /* 5. eraseable list */
    if(!_eraseable_list.empty())
      if(find(_eraseable_list.begin(), _eraseable_list.end(), b) != _eraseable_list.end())
	occurences++;

    /* 6. erase pending */
    if(!_erase_pending.empty())
      if(find(_erase_pending.begin(), _erase_pending.end(), b) != _erase_pending.end())
    	occurences++;

    /* 7. erase complete */
    if(!_erase_complete.empty())
      if(find(_erase_complete.begin(), _erase_complete.end(), b) != _erase_complete.end())
	occurences++;

    /* 8 dirty */
    if(!_dirty_list.empty())
      if(find(_dirty_list.begin(), _dirty_list.end(), b) != _dirty_list.end())
	occurences++;

    /* 9. very dirty */
    if(!_very_dirty_list.empty())
      if(find(_very_dirty_list.begin(), _very_dirty_list.end(), b) != _very_dirty_list.end())
    	occurences++;

    if(occurences != 1)
      assert(0);
  }

  return 0;
}

bool Jffs2GC::gcdShouldWake ()
{
  int res = false;
  //TODO

  if(_erase_complete.size() > 0 || _erase_pending.size() > 0)
    return true;

  /* compute dirty space */
  uint64_t dirty = 0;
  for(int i=0; i<(int)_dirty_list.size(); i++)
    dirty += _dirty_list[i]->getDirtySize();
  for(int i=0; i<(int)_very_dirty_list.size(); i++)
    dirty += _very_dirty_list[i]->getDirtySize();

  if((int)_free_list.size() < _resv_blocks_gc_trigger && dirty > _no_spc_dirty_size)
    return true;

  if((int)_very_dirty_list.size() >= _very_dirty_blocks_gc_trigger)
    return true;

  return res;
}

int Jffs2GC::gcdTrigger ()
{
  assert(!_gc_in_progress);

  if(_gcd_enabled && !_gcd_scheduled)
    if(gcdShouldWake())
    {
      _gcd_scheduled = true;
      Jffs2GcThread::insert();
    }

  return 0;
}

void Jffs2GC::gcdDone()
{
  assert(_gcd_scheduled);
  _gcd_scheduled = false;
}

void Jffs2GC::printBlockLists ()
{
  cout << "free_list : ";
  for(int i=0; i<(int)_free_list.size(); i++)
    cout << _free_list[i]->getIndex() << ", ";
  cout << endl;

  cout << "dirty_list : ";
  for(int i=0; i<(int)_dirty_list.size(); i++)
    cout << _dirty_list[i]->getIndex() << ", ";
  cout << endl;

  cout << "very_dirty_list : ";
  for(int i=0; i<(int)_very_dirty_list.size(); i++)
    cout << _very_dirty_list[i]->getIndex() << ", ";
  cout << endl;

  cout << "clean_list : ";
  for(int i=0; i<(int)_clean_list.size(); i++)
    cout << _clean_list[i]->getIndex() << ", ";
  cout << endl;

  cout << "eraseable_list : ";
  for(int i=0; i<(int)_eraseable_list.size(); i++)
    cout << _eraseable_list[i]->getIndex() << ", ";
  cout << endl;

  cout << "erase_pending_list : ";
  for(int i=0; i<(int)_erase_pending.size(); i++)
    cout << _erase_pending[i]->getIndex() << ", ";
  cout << endl;

  cout << "erase_complete_list : ";
  for(int i=0; i<(int)_erase_complete.size(); i++)
    cout << _erase_complete[i]->getIndex() << ", ";
  cout << endl;
}
