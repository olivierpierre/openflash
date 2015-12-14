/*
 * Jffs2File.hpp
 *
 *  Created on: 13 mars 2014
 *      Author: pierre
 */

#ifndef JFFS2FILE_HPP
#define JFFS2FILE_HPP

#include <stdint.h>
#include <vector>

#include "../../../FlashLayer/Address.hpp"

using namespace std;

/**
 * Jffs2 Node Model
 */

#define SIZEOF_JFFS2_RAW_INODE		68	/* size of a data node header, it is sizeof(jffs2_raw_inode) */
#define SIZEOF_JFFS2_RAW_DIRENT		40	/* size of a dentry header, sizeof(jffs2_raw_dirent) */

typedef enum
{
  JFFS2_DATA,
  JFFS2_DIRENT
}jffs2_node_t;

typedef enum
{
  JFFS2_NODE_VALID,
  JFFS2_NODE_INVALID
}jffs2_node_state_t;

class Jffs2GC;

class Jffs2Node
{
public:
  Jffs2Node(uint32_t inode_num, uint32_t version, Jffs2GC *gc);
  virtual jffs2_node_t getType()const=0;
  virtual uint32_t getFlashSize()const=0;
  virtual ~Jffs2Node();
  uint32_t getVersion() const {return _version;};
  uint32_t getInodeNum() const {return _inode_num;};
  uint64_t getFlashOffset() const {return _flash_offset;};
  jffs2_node_state_t getState() {return _state;};

protected:
  uint32_t _inode_num;		// Data / dirent node related inode number
  uint32_t _version;		// Data / dirent node version
  uint64_t _flash_offset;	// location on flash, will be set by the writebuffer
  jffs2_node_state_t _state;	// Node valid / invalid state
  Jffs2GC *_gc;			// pointer on the gc
  int invalidate();		// the node must call this when invalidated to set its state and re-classify the containing block

  friend class Jffs2WriteBuffer;
  friend ostream& operator<<(ostream& os, const Jffs2Node& n);  /**< print on stream */

};

class Jffs2DataNode : public Jffs2Node
{
public:
  Jffs2DataNode(uint32_t inode_num, uint32_t version, uint32_t data_offset,
                uint32_t data_size, uint32_t file_size, Jffs2GC *gc);
  jffs2_node_t getType() const { return JFFS2_DATA;};
  uint32_t getFlashSize() const {return _data_size + SIZEOF_JFFS2_RAW_INODE;};
  uint32_t getFileSize() {return _file_size;};
  uint32_t getDataOffset() {return _data_offset;};
  uint32_t getDataSize() {return _data_size;};
  virtual ~Jffs2DataNode();

  int incrementFragCount();
  int decrementFragCount();
  int invalidateZeroSizedNode();

private:
  uint32_t _data_offset;	// location of the stored data in file
  uint32_t _data_size;		// size of stored data
  uint32_t _file_size;		// total size of the file at the time the node is written

  int _frag_count;		// number of frags pointing to this data node 0 --> node fully obsolete

  friend class Jffs2WriteBuffer;
  friend ostream& operator<<(ostream& os, const Jffs2DataNode& dn);  /**< print on stream */

};

class Jffs2DirentNode : public Jffs2Node
{
public:
  Jffs2DirentNode(uint32_t inode_num, uint32_t version, uint16_t name_len, Jffs2GC *gc);
  jffs2_node_t getType() const { return JFFS2_DIRENT;};
  uint32_t getFlashSize() const {return _name_len + SIZEOF_JFFS2_RAW_DIRENT;};
  virtual ~Jffs2DirentNode();
  uint16_t getNameLen() {return _name_len;};

  void setDeletionDirent();
  void setRenameDirent();
  bool isDeletion() {return _deletion_dirent;};
  bool isRename() {return _rename_dirent;};

  int invalidateOldDirent();

private:
  uint16_t _name_len;		// name lenght in bytes
  bool _deletion_dirent;
  bool _rename_dirent;

  friend class Jffs2WriteBuffer;
  friend ostream& operator<<(ostream& os, const Jffs2DirentNode& dn);  /**< print on stream */

};

typedef enum
{
  JFFS2_FRAGS_NO_OVERLAP,
  JFFS2_FRAGS_PARTIAL_LEFT,
  JFFS2_FRAGS_PARTIAL_RIGHT,
  JFFS2_FRAGS_SPLIT,
  JFFS2_FRAGS_TOTAL_OVERLAP
}jffs2_frags_comp_t;

class Jffs2Frag
{
public:
  Jffs2Frag(uint32_t file_offset, uint32_t size, Jffs2DataNode *data_node);
  uint32_t getFileOffset() const {return _file_offset;};
  uint32_t getSize() const {return _size;};
  void setFileOffset(uint32_t file_offset) {_file_offset = file_offset;};
  void setSize(uint32_t size) {_size = size;};
  Jffs2DataNode *getDataNodePtr() {return _data_node;};

  jffs2_frags_comp_t compare(const Jffs2Frag &other) const;

private:
  uint32_t _file_offset;
  uint32_t _size;
  Jffs2DataNode *_data_node;

friend ostream& operator<<(ostream& os, const Jffs2Frag& f);  /**< print on stream */

};

/**
 * Jffs2 File Model
 */

class Jffs2File
{
public:
  Jffs2File (uint32_t inode_num);
  virtual ~Jffs2File ();
  Jffs2DataNode * addDataNode(Jffs2DataNode &dn);
  Jffs2DirentNode * addDirentNode(Jffs2DirentNode &dn);
  int deleteNode(Jffs2Node *n);
  uint32_t getInodeNum() const;
  uint32_t getLastDataNodeVersion();
  uint32_t getLastDirentNodeVersion();
  uint16_t getNameLen() const;
  uint32_t getSize() const;

private:
  uint32_t _inode_num;
  vector<Jffs2DataNode *> _data_nodes;
  vector<Jffs2Frag *> _frags;
  vector<Jffs2DirentNode *> _dirent_nodes;

  int addFrag(Jffs2Frag &frag);
  int delFragAtIndex(int index);
  int insertFragOrdered(Jffs2Frag *frag);

  //debug functions
  void fragSanityCheck();
  void printFragMap();

  friend class Jffs2;
  friend ostream& operator<<(ostream& os, const Jffs2File& f);  /**< print on stream */

};

/**
 * More info about fragment comparison :
 *
 * +----- address in file---->
 *
 * I. No overlap at all :
 *                   +---------------+
 * +--------------+  |               |
 * |   other      |  |    *this      |
 * +--------------+  |               |
 *                   +---------------+
 *
 * II. Partial overlap on the left side
 *        +----------+
 *        |       +--|-----------+
 *        |*this  |  |other      |
 *        |       +--|-----------+
 *        +----------+
 *
 * III. Partial overlap on the right side
 *                           +-------------+
 *                +----------|---+         |
 *                |   other  |   | *this   |
 *                +----------|---+         |
 *                           +-------------+
 *
 * IV. Split
 *                       +----------------+
 *          +------------|----------------|----+
 *          | other      |    *this       |    |
 *          +------------|----------------|----+
 *                       +----------------+
 *
 * V. Total overlap
 *    +--------------------+
 *    |--------------------|
 *    |  *this & other     |
 *    |--------------------|
 *    +--------------------+
 */

#endif /* JFFS2FILE_HPP */
