/*
 * Jffs2File.cpp
 *
 *  Created on: 13 mars 2014
 *      Author: pierre
 */

#include "Jffs2File.hpp"
#include "../VFS/VFS.hpp"
#include <assert.h>
#include <iostream>
#include "../../../Globs/Common.hpp"


using namespace std;

/********************************
 *      Node model              *
 *******************************/
Jffs2Node::Jffs2Node(uint32_t inode_num, uint32_t version, Jffs2GC *gc) : _inode_num(inode_num), _version(version)
{
  _flash_offset = 0xFFFFFFFFFFFFFFFF;
  _state = JFFS2_NODE_VALID;
  _gc = gc;
}

Jffs2DataNode::Jffs2DataNode (uint32_t inode_num, uint32_t version,
			      uint32_t data_offset, uint32_t data_size,
			      uint32_t file_size, Jffs2GC *gc) :
			      Jffs2Node(inode_num, version, gc),
			      _data_offset(data_offset), _data_size(data_size),
			      _file_size(file_size), _frag_count(0) {}

Jffs2DirentNode::Jffs2DirentNode (uint32_t inode_num, uint32_t version,
				  uint16_t name_len, Jffs2GC *gc) :
				      Jffs2Node(inode_num, version, gc),
				      _name_len(name_len), _deletion_dirent(false), _rename_dirent(false) {}

Jffs2Node::~Jffs2Node() {}

int Jffs2Node::invalidate ()
{
  _state = JFFS2_NODE_INVALID;
  _gc->reclassifyBlockAtFlashOffset(_flash_offset);
  return 0;
}

ostream& operator<<(ostream& os, const Jffs2Node& n)
{
  if(n.getType() == JFFS2_DATA)
    os << "Da";
  else if(n.getType() == JFFS2_DIRENT)
    os << "Di";
  else
    ERROR("wrong jffs2 Node type");

  os << n.getInodeNum() << "v" << n.getVersion();

  return os;
}

Jffs2DataNode::~Jffs2DataNode() {}

Jffs2DirentNode::~Jffs2DirentNode() {}


int Jffs2DataNode::incrementFragCount ()
{
  _frag_count++;
  return _frag_count;
}

int Jffs2DataNode::decrementFragCount ()
{
  _frag_count--;
  assert(_frag_count >= 0);

  if(_frag_count == 0)
    invalidate();

  /** TODO here check if the block is fully invalid, move it to erase  pending / erasable list
   * if it is the case
   * Do nothing if it is the current block
   */

  return _frag_count;
}

/**
 * As a zero size dnode has no frags, we cannot use fragcount == 0 to say its is valid / invalid
 * So this function should be called each time we add a data node for a file which last
 * data node is 0 sized
 */
int Jffs2DataNode::invalidateZeroSizedNode ()
{
  assert(_data_size == 0 && _data_offset == 0 && _file_size == 0);
  assert(_frag_count == 0);

  invalidate();
  return 0;
}

int Jffs2DirentNode::invalidateOldDirent ()
{
  invalidate();
  return 0;
}

ostream& operator<<(ostream& os, const Jffs2DataNode& dn)
{
  os << "DataNode " << dn._inode_num << "v" << dn._version << ", flash_offset:" <<
      dn.getFlashOffset() << ", flash size:" << dn.getFlashSize() << " data size:" <<
      dn._data_size << " ";
  if(dn._state == JFFS2_NODE_VALID)
    os << "state: valid";
  else
    os << "state: invalid";

  return os;
}

/**
 * Indicate that this dirent node represent the deletion of a file
 */
void Jffs2DirentNode::setDeletionDirent ()
{
  assert(_inode_num == 0);
  assert(_rename_dirent == false);

  _deletion_dirent = true;
}

/**
 * Indicate that this dirent node represent a file renaming operation
 */
void Jffs2DirentNode::setRenameDirent ()
{
  assert(_inode_num == 0);
  assert(_deletion_dirent == false);

  _rename_dirent = true;
}

ostream& operator<<(ostream& os, const Jffs2DirentNode& dn)
{
  os << "DirentNode " << dn._inode_num << "v" << dn._version << ", flash_offset:" <<
      dn.getFlashOffset() << ", flash size:" << dn.getFlashSize() << " ";
  if(dn._state == JFFS2_NODE_VALID)
    os << "state: valid";
  else
    os << "state: invalid";

  return os;
}


/********************************
 *        Frag model            *
 *******************************/

Jffs2Frag::Jffs2Frag (uint32_t file_offset, uint32_t size, Jffs2DataNode* data_node) :
			  _file_offset(file_offset), _size(size), _data_node(data_node) {}

ostream& operator<<(ostream& os, const Jffs2Frag& f)
{
    os << "frag [" << f._data_node->getInodeNum() << "v" << f._data_node->getVersion() << "] @" <<
	f._file_offset << "->" << f._file_offset+f._size-1;
    return os;
}

/********************************
 *      File model              *
 *******************************/
Jffs2File::Jffs2File (uint32_t inode_num) : _inode_num(inode_num){}

Jffs2File::~Jffs2File ()
{
  for(int i=0; i<(int)_data_nodes.size(); i++)
    delete(_data_nodes[i]);

  for(int i=0; i<(int)_dirent_nodes.size(); i++)
    delete(_dirent_nodes[i]);

  for(int i=0; i<(int)_frags.size(); i++)
    delete(_frags[i]);
}

/**
 * Add a data node to the data node list of the file
 * Return a pointer to the stored data node
 */
Jffs2DataNode * Jffs2File::addDataNode(Jffs2DataNode &dn)
{
  /* sanity checks */
  assert(dn.getInodeNum() == _inode_num);
  if(!_data_nodes.empty())
    assert(dn.getVersion() > _data_nodes.back()->getVersion());
  assert(!(dn.getDataSize()==0 && dn.getDataOffset()!= 0));

#ifdef VERBOSE
  cout << "[JFFS2:" << TIME_NOW << "] Added DataNode " << dn.getInodeNum() << "v" << dn.getVersion() <<
      " [";
  if(dn.getDataSize() == 0)
    cout << "no_data";
  else
    cout << dn.getDataOffset() << "->" << (dn.getDataOffset()+dn.getDataSize()-1);
  cout << "], filesize:" << dn.getFileSize() << endl;
#endif

  Jffs2DataNode *data_node = new Jffs2DataNode(dn);
  _data_nodes.push_back(data_node);

  if(data_node->getDataSize() == 0)
  {
    // its a truncation to size 0, delete all existing frags
#ifdef VERBOSE
    cout << "[JFFS2] Truncation to size 0 or first 0 sized datanode, deleting all current frags (" <<
	_frags.size() << ")" << endl;
#endif /* VERBOSE */
    while(!_frags.empty())
      delFragAtIndex(0);
  }
  else
  {
    // its a normal write, add frag
    Jffs2Frag frag(data_node->getDataOffset(), data_node->getDataSize(), data_node);
    addFrag(frag);
  }

  // if the last data node of that file is 0 sized, it can be obsoleted because
  // the newly inserted node necessarily invalidates it
  if(_data_nodes.size() > 1)
  {
    Jffs2DataNode &last_dn = *_data_nodes[_data_nodes.size()-2];
    if(last_dn.getFileSize() == 0 && last_dn.getDataSize() == 0)
      last_dn.invalidateZeroSizedNode();
  }

  return data_node;
}

/**
 * Same as above for dirent
 */
Jffs2DirentNode * Jffs2File::addDirentNode(Jffs2DirentNode &dn)
{
  assert(dn.getInodeNum() == _inode_num || !dn.getInodeNum());	// it can be 0 when deleting the file
  if(!_dirent_nodes.empty())
    assert(dn.getVersion() > _dirent_nodes.back()->getVersion());

  Jffs2DirentNode *dirent_node = new Jffs2DirentNode(dn);
  _dirent_nodes.push_back(dirent_node);

#ifdef VERBOSE
  cout << "[JFFS2] Added DirentNode " << dn.getInodeNum() << "v" << dn.getVersion() << endl;
#endif

  /* Invalidate last dirent node if any */
  if(_dirent_nodes.size() > 1)
    _dirent_nodes[_dirent_nodes.size() - 2]->invalidateOldDirent();

  /* is this a file deletion ? */
  if(!dn.getInodeNum() && dn.isDeletion())
  {
    // delete all frags, it will implicitely invalidate all data nodes
    while(!_frags.empty())
      delFragAtIndex(0);

    //TODO what do we do with the last dentry we just inserted ?

  }

  return dirent_node;
}

uint32_t Jffs2File::getInodeNum () const
{
  return _inode_num;
}

/**
 * Returns the last data node version (most recent).
 */
uint32_t Jffs2File::getLastDataNodeVersion ()
{
  if(_data_nodes.empty())
    ERROR("Requesting last data node version on "
    	"a JFFS2 file (" + nts(_inode_num) + ") with no data node");

  return _data_nodes.back()->getVersion();
}

/**
 * Returns the last dirent version
 */
uint32_t Jffs2File::getLastDirentNodeVersion ()
{
  if(_dirent_nodes.empty())
    ERROR("Requesting last dirent node version on "
	"a JFFS2 file (" + nts(_inode_num) + ") with no dirent node");

  return _dirent_nodes.back()->getVersion();
}

uint16_t Jffs2File::getNameLen () const
{
  if(_dirent_nodes.empty())
    ERROR("Requesting name len on "
    	"a JFFS2 file (" + nts(_inode_num) + ") with no dirent node");
  return _dirent_nodes.back()->getNameLen();
}

uint32_t Jffs2File::getSize() const
{
  uint32_t res;

  if(_data_nodes.empty())
  {
    WARNING("Requesting file size on a JFFS2 file "
	"(" + nts(_inode_num) + ") with no data node");
    return 0;
  }

  if(_data_nodes.back()->getState() == JFFS2_NODE_VALID)
    res = _data_nodes.back()->getFileSize();
  else
    res = 0;

  return res;
}

ostream& operator<<(ostream& os, const Jffs2File& f)
{
  bool no_data_nodes = (f._data_nodes.empty());
  bool no_dirent_nodes = (f._dirent_nodes.empty());

  os << "Jffs2File ino:" << f.getInodeNum() << ", size:";
  if(no_data_nodes || f._data_nodes.back()->getState() == JFFS2_NODE_INVALID)
    os << "(no data nodes)";
  else
    os << f.getSize();

  os << ", nlen:";

  if(no_dirent_nodes)
    os << "(no dirent nodes)";
  else
    os << f.getNameLen();
  return os;
}

/**
 * Add a new frag to the frag list, delete potential obsolete frags,
 * update nodes frag counters for obsoleted nodes & the node corresponding
 * to the added frag
 */
int Jffs2File::addFrag(Jffs2Frag& frag)
{
  int res = 0;

  /**
   * When adding a new frag, compare the range of that frag to every other frag
   * of the file (maybe make something like a tree to speedup that process).
   * So the added frag is compared to all the other frags to see if they overlap.
   * During one comaprison 4 cases can append :
   *  1. No overlaping -> nothing to do, next frag
   *  2. Partial overlaping left / right by the new frag for the old frag
   *    - Reduce the offset / size for the old frag
   *    - insert the new frag
   *    - increment the node for the new frag frag count
   *  3. Split of the old frag by the new frag
   *    - create a new frag corresponding to the second part of the old frag, isnert it
   *    - insert the new frag
   *    - increment the node frag count for the old frag, and increment for the new one
   *  4. Total overlapping by the new frag
   *    - delete the overlapped frag, decrement corresponding node frag count
   *    - insert new frag, increment corresponding node frag count
   *
   * FIXME this function is particularry ugly. Iterating over the _frags vector we cannot
   * modify it (remove or add elements) because it mess up the iteration. So there are 2 vectors
   * containing the frags to delete (total overlap) or add (split) in addition to the newly created
   * frag. At the end of the comparison of the new frag with all the existing frags
   * (that could also be optimized) we have to iterate over these two arrays to actually delete and
   * add new fragments
   */

#ifdef VERBOSE
  cout << "[JFFS2] Adding frag for file " << _inode_num << ", @" << frag.getFileOffset() <<
      "->@" << frag.getFileOffset()+frag.getSize()-1 << endl;
#endif /* VERBOSE */

  Jffs2Frag *new_frag = new Jffs2Frag(frag);
  vector<Jffs2Frag *> frags_to_delete;
  vector<Jffs2Frag *> frags_to_add;
  for(int i=0; i<(int)_frags.size(); i++)
  {
    Jffs2Frag *cur_frag = _frags[i];
    switch(new_frag->compare(*cur_frag))
    {
      case JFFS2_FRAGS_NO_OVERLAP:
	continue;
	break;

      case JFFS2_FRAGS_TOTAL_OVERLAP:
#ifdef VERBOSE
	cout << "[JFFS2] New " << *new_frag << " totally overlaps old " << *cur_frag <<
	", deleting old." << endl;
#endif /* VERBOSE */
	//delFragAtIndex(i);
	frags_to_delete.push_back(_frags[i]);
	break;

      case JFFS2_FRAGS_PARTIAL_LEFT:
      {
#ifdef VERBOSE
	cout << "[JFFS2] New " << *new_frag << " overlaps left " << *cur_frag << endl;
#endif /* VERBOSE */
	int cur_old_file_offset = cur_frag->getFileOffset();
	int cur_old_end = cur_old_file_offset + cur_frag->getSize();
	int cur_new_file_offset = new_frag->getFileOffset() + new_frag->getSize();
	cur_frag->setFileOffset(cur_new_file_offset);
	cur_frag->setSize(cur_old_end - cur_new_file_offset);
	break;
      }

      case JFFS2_FRAGS_PARTIAL_RIGHT:
#ifdef VERBOSE
	cout << "[JFFS2] New " << *new_frag << " overlaps right " << *cur_frag << endl;
#endif /* VERBOSE */
	cur_frag->setSize(new_frag->getFileOffset() - cur_frag->getFileOffset());
	break;

      case JFFS2_FRAGS_SPLIT:
      {
#ifdef VERBOSE
	cout << "[JFFS2] New " << *new_frag << " splits " << *cur_frag << " in : ";
#endif /* VERBOSE */
	int old_size = cur_frag->getSize();
	cur_frag->setSize(new_frag->getFileOffset() - cur_frag->getFileOffset());
	Jffs2Frag *splitted = new Jffs2Frag(new_frag->getFileOffset()+new_frag->getSize(),
	                                    old_size-(cur_frag->getSize()+new_frag->getSize()),
	                                	cur_frag->getDataNodePtr());
	//insertFragOrdered(splitted);
	//splitted->getDataNodePtr()->incrementFragCount();
	frags_to_add.push_back(splitted);
#ifdef VERBOSE
	    cout << *cur_frag << " and " << *splitted << endl;
#endif /* VERBOSE */
	break;
      }

      default:
	ERROR("Error in frag comparison");
	break;
    }
  }

  // delete what needs to be deleted
  for(int i=0; i<(int)frags_to_delete.size(); i++)
    for(int j=0; j<(int)_frags.size(); j++)
      if(_frags[j] == frags_to_delete[i])
      {
	delFragAtIndex(j);
	break;
      }

  // insert frags created by split
  for(int i=0; i<(int)frags_to_add.size(); i++)
  {
    insertFragOrdered(frags_to_add[i]);
    frags_to_add[i]->getDataNodePtr()->incrementFragCount();
  }

  // Finally insert newly created frag
  insertFragOrdered(new_frag);
  new_frag->getDataNodePtr()->incrementFragCount();

#ifndef NDEBUG
  fragSanityCheck();
#endif /* NDEBUG */

  return res;
}

/**
 * Compare 2 frags
 * The result is related to *this toward other :
 * ex : if the result is "total overlap", it means *this
 * totally overlaps other. If it is "partial left", it means *this
 * overlaps other on the left side
 */
jffs2_frags_comp_t Jffs2Frag::compare (const Jffs2Frag& other) const
{
  uint32_t start1 = _file_offset;
  uint32_t end1 = start1 + _size -1;
  uint32_t start2 = other.getFileOffset();
  uint32_t end2 = start2 + other.getSize() - 1;

  if(start1 <= start2 && end1 >= end2)
    return JFFS2_FRAGS_TOTAL_OVERLAP;

  if(start1 <= start2 && end1 < end2 && end1 >= start2)
    return JFFS2_FRAGS_PARTIAL_LEFT;

  if(start1 > start2 && start1 <= end2 && end1 >= end2)
    return JFFS2_FRAGS_PARTIAL_RIGHT;

  if(start1 > start2 && end1 < end2)
    return JFFS2_FRAGS_SPLIT;

  return JFFS2_FRAGS_NO_OVERLAP;
}

/**
 * Delete the frag at index 'index' in the file frag list :
 * 1. decrement corresponding node frag counter
 * 2. free the frag pointer
 * 3. remove the frag from the frag list
 */
int Jffs2File::delFragAtIndex (int index)
{
  Jffs2Frag *frag = _frags[index];

  frag->getDataNodePtr()->decrementFragCount();
  delete frag;
  _frags.erase(_frags.begin()+index);

  return 0;
}

/**
 * Insert a frag in the corresponding place in the frag list (based on its start offset)
 * This method should only be called by File::addFrag, with all the invalidation / split
 * already done
 */
int Jffs2File::insertFragOrdered (Jffs2Frag* frag)
{
  bool inserted = false;

  for(int i=0; i<(int)_frags.size(); i++)
    if((_frags[i])->getFileOffset() > frag->getFileOffset())
    {
      _frags.insert(_frags.begin() + i, frag);
      inserted = true;
      break;
    }
  if(!inserted)
    _frags.push_back(frag);

  return 0;
}

void Jffs2File::fragSanityCheck ()
{
  uint32_t offset = 0;

  for(int i=0; i<(int)_frags.size(); i++)
  {
    Jffs2Frag *frag = _frags[i];
    if(frag->getFileOffset() != offset)
    {
      printFragMap();
      assert(0);
    }
    offset += frag->getSize();
  }
}

/**
 * Delete the corresponding node from fs metadata. Free the corresponding memory.
 * If it deletes the last node of a file, this function returns 1 (0 otherwise)
 * so the system knows it can detroy the file object
 */
int Jffs2File::deleteNode (Jffs2Node* n)
{
  bool ok = false;
  /* keep gcc happy when NDEBUG */
  (void)ok;

  if(n->getType() == JFFS2_DATA)
  {
    for(int i=0; i<(int)_data_nodes.size(); i++)
      if(_data_nodes[i] == n)
      {
	delete _data_nodes[i];
	_data_nodes.erase(_data_nodes.begin()+i);
	ok = true;
	break;
      }
  }
  else
  {
    for(int i=0; i<(int)_dirent_nodes.size(); i++)
      if(_dirent_nodes[i] == n)
      {
	delete _dirent_nodes[i];
	_dirent_nodes.erase(_dirent_nodes.begin()+i);
	ok = true;
	break;
      }
  }

  assert(ok);

  if(_dirent_nodes.size() == 0 && _data_nodes.size() == 0)
    return 1;
  return 0;
}

void Jffs2File::printFragMap ()
{
  uint32_t offset = 0;

  cout << "FILEMAP (" << _inode_num << "): ";

  for(int i=0; i<(int)_frags.size(); i++)
  {
    Jffs2Frag & frag = *(_frags[i]);
    if(frag.getFileOffset() != offset)
      cout << "|||||||||||||||||||||||||||||||||";
    cout << frag.getFileOffset() << "->" << frag.getFileOffset() + frag.getSize() -1 << ", ";
    offset = frag.getFileOffset() + frag.getSize();
  }
  cout << endl;
}
