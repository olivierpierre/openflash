#include "Jffs2.hpp"
#include "../VFS/VFS.hpp"
#include "../VFS/PageCache.hpp"

Jffs2::Jffs2() : FlashFileSystem(FlashSystem::getInstance(), NandDriver::getInstance(), PageCache::getInstance())
{
  _base_readpage_timing_overhead = Param::getInstance()->getDouble("functional_model.ffs.jffs2.base_readpage_timing_overhead");
  _base_write_end_timing_overhead = Param::getInstance()->getDouble("functional_model.ffs.jffs2.base_write_end_timing_overhead");
  _base_write_begin_timing_overhead = Param::getInstance()->getDouble("functional_model.ffs.jffs2.base_write_begin_timing_overhead");

  _read_check = Param::getInstance()->getBool("functional_model.ffs.jffs2.read_check");


  // create wbuf object
  _wbuf = new Jffs2WriteBuffer(this, FlashSystem::getInstance()->getStructuralParameter(SP_PAGE_SIZE_BYTE));
  _gc = new Jffs2GC(this);

  _wbuf->updateCurrentBlock();
  _stats = new Jffs2Stats(Param::getInstance()->isParamPresent("outputs.jffs2"));
}

Jffs2::~Jffs2()
{
  /* free all priv data if needed */
  for(int channel=0; channel < (int)CHANNEL_NUM; channel++)
    for(int lun=0; lun < (int)LUNS_PER_CHANNEL; lun++)
      for(int plane=0; plane<(int)PLANES_PER_LUN; plane++)
	for(int block=0; block<(int)BLOCKS_PER_PLANE; block++)
	  for(int page=0; page<(int)PAGES_PER_BLOCK; page++)
	  {
	    vector<Jffs2Node *> *priv =
		(vector<Jffs2Node *> *)_f->getPagePriv(Address(channel, lun, plane, block, page));
	    if(priv != NULL)
	      delete priv;
	  }

  delete _wbuf;
  delete _gc;
  delete _stats;

  /* delete all files */
  for(int i=0; i<(int)_files.size(); i++)
    delete _files[i];
}

flash_file_system_t Jffs2::getType()
{
  return JFFS2;
}

PpcValF Jffs2::ffsReadPage (uint32_t inode_num, uint32_t page_index, int *ret)
{
 PpcValF res = { 0, 0 };
 int frags_read = 0;
 Jffs2File *f = getFile(inode_num);

 res.time += _base_readpage_timing_overhead;

 if(f == NULL)
   ERROR("Jffs2ReadPage called on non existent file (ino=" + nts(inode_num) + ")");

 // It can happen, for example when we write at the end of a file, jffs2_write_begin
 // reads the page in the page cache if it is not present. If we write to a new page
 // there is nothing to read
 if(page_index*LINUX_PAGE_SIZE >= f->getSize())
 {
//   em->warning("Calling jffs2 readpage outside of file (asked @" + nts(page_index*LINUX_PAGE_SIZE)
//             + ", but file has size " + nts(f->getSize()));
#ifdef VERBOSE
   cout << "[jffs2] nothing to read for read page " << page_index << " , ino:" << inode_num << endl;
#endif /* VERBOSE */
   *ret = 0;
   return res;
 }

#ifdef VERBOSE
  vector<Jffs2Frag *> frags_read_log;
  cout << "[JFFS2] readpage ino:" << inode_num << ", page index:" << page_index << endl;
#endif /* VERBOSE */

 for(int i=0; i<(int)f->_frags.size(); i++)
 {
   Jffs2Frag *frag = f->_frags[i];

   if(frag->getFileOffset() < page_index*LINUX_PAGE_SIZE)
     continue;
   if(frag->getFileOffset() >= (page_index+1)*LINUX_PAGE_SIZE)
     break;

   frags_read++;

#ifdef VERBOSE
   frags_read_log.push_back(f->_frags[i]);
#endif /* VERBOSE */

   Jffs2DataNode *dn = frag->getDataNodePtr();
   vector<Address> frag_pages_to_read = _d->byteRangeToPages(dn->getFlashOffset(), dn->getFlashSize());
   for(int j=0; j<(int)frag_pages_to_read.size(); j++)
   {
     //the page may be in the writebuffer
     Address wbuf_addr = _wbuf->getAddress();
     if(frag_pages_to_read[j] == wbuf_addr)
     {
       assert(!_wbuf->empty());
       continue;
     }

     res = res + _d->readPage(frag_pages_to_read[j], MTD_SRC_JFFS2_READPAGE);

     // check priv data
     if(_read_check)
     {
       vector<Jffs2Node *> *priv = (vector<Jffs2Node *> *)_f->getPagePriv(frag_pages_to_read[j]);
       bool checked = false;
       for(int k=0; k<(int)priv->size(); k++)
	 if(priv->at(k)->getVersion() == dn->getVersion())
	 {
	   checked = true;
	   break;
	 }
       if(!checked)
	 ERROR("JFFS2 read check on datanode version failed, expected " + nts(dn->getVersion()) + ".");
     }
   }
 }

#ifdef VERBOSE
  cout << "[JFFS2] Frags read : ";
  for(int i=0; i<(int)frags_read_log.size(); i++)
    cout << frags_read_log[i]->getFileOffset() << "->" << frags_read_log[i]->getFileOffset()+frags_read_log[i]->getSize()-1 << ", ";
  cout << endl;
#endif /* VERBOSE */

 assert(frags_read > 0);
 *ret = LINUX_PAGE_SIZE;



 _stats->addJffs2CallTimingInfo(JFFS2_READPAGE, res.time, res.e_cpu, res.e_mem);

 return res;
}

/**
 * here offset is the byte offset in the page where the write begin
 */
PpcValF Jffs2::ffsWriteBegin(uint32_t inode_num, uint32_t page_index, uint32_t offset, uint32_t count)
{
  PpcValF res = {0, 0};

#ifdef VERBOSE
  cout << "[JFFS2:" << TIME_NOW << "] write begin ino;" << inode_num
      << ", page:" << page_index << ", offset:" << offset <<
      ", count:" << count << endl;
#endif /* VERBOSE */

   //Read the page if not present in the page cache
  if(PageCache::getInstance()->findInPageCache(inode_num, page_index) == NULL)
  {
    int ret;
    res = res + ffsReadPage(inode_num, page_index, &ret);
    if(ret)
      _page_cache->insertInPageCache(inode_num, page_index);
    // ignore i/o errors
  }
  else
    res.time = res.time + _base_write_begin_timing_overhead;


  _stats->addJffs2CallTimingInfo(JFFS2_WRITE_BEGIN, res.time, res.e_cpu, res.e_mem);

  return res;
}

/**
 * here offset is the byte offset in the page where the write begin
 */
PpcValF Jffs2::ffsWriteEnd(uint32_t inode_num, uint32_t page_index, uint32_t offset, uint32_t count)
{
  PpcValF res = {0, 0};
  Jffs2File *f = getFile(inode_num);

#ifdef VERBOSE
  cout << "[JFFS2:" << TIME_NOW << "] Jffs2WriteEnd ino:" << inode_num <<
      ", page_index:" << page_index <<
      ", offset:" << offset << ", count:" << count << endl;
#endif /* VERBOSE */

  res.time += _base_write_end_timing_overhead;

  if(f == NULL)
    ERROR("Jffs2WriteEnd called on non existent file (ino=" + nts(inode_num) + ")");

  if(!(count > 0))
    ERROR("Jffs2WriteEnd received 0 sized write ...");

  // If we are writing the end of a linux page, rewrite the entire page
  if(offset + count == LINUX_PAGE_SIZE)
  {
    offset = 0;
    count = LINUX_PAGE_SIZE;
  }

  // reserve space for max 2 nodes
  res = res + reserveSpace(count + 2*SIZEOF_JFFS2_RAW_INODE);


  /* If there is not enough space in the current block to store the node header and at least
   * 1 byte of data, flush the buffer and we will get a new current block */
#ifndef NDEBUG
  int old_current_block = _wbuf->getAddress().getBlock();
#endif /* NDEBUG */
  if(_wbuf->getFreeSpaceLeftInCurrentBlock() < (SIZEOF_JFFS2_RAW_INODE+1))
  {
    res = res + _wbuf->flush(MTD_SRC_JFFS2_WRITE_END);
    assert(old_current_block != _wbuf->getAddress().getBlock());
  }

  /**
   * If there is enough free space in the current block only create one data node of
   * size count. If there is not enough free space create two data nodes, the first filling the
   * current block as far as we can
   */
  if(_wbuf->getFreeSpaceLeftInCurrentBlock() >= count+SIZEOF_JFFS2_RAW_INODE)
  {
    // We have enough space to perform the write in one node

    // compute new file size if it grows
    uint32_t end_byte_offset = (page_index*LINUX_PAGE_SIZE + offset + count);
    uint32_t new_file_size = max(end_byte_offset, f->getSize());
    //create new datanode
    Jffs2DataNode dn(inode_num, f->getLastDataNodeVersion()+1,
                     page_index*LINUX_PAGE_SIZE+offset, count, new_file_size, _gc);
    //add it to the FS metadata
    Jffs2DataNode *dn_ptr = f->addDataNode(dn);
    //write on flash through wbuf
    res = res + _wbuf->writeNode(static_cast<Jffs2DataNode *>(dn_ptr), MTD_SRC_JFFS2_WRITE_END);
  }
  else
  {
    // ok we dont have enough space in the current block, fill the current as far as we can, then ask
    // wbuf to pick a new block and write into it a new node containing the rest of the data

    // node 1.
      // set new size if the file grows
    uint32_t node1_count = _wbuf->getFreeSpaceLeftInCurrentBlock() - SIZEOF_JFFS2_RAW_INODE;
    uint32_t end_byte_offset = (page_index*LINUX_PAGE_SIZE + offset + node1_count);
    uint32_t new_file_size = max(end_byte_offset, f->getSize());
    // create datanode 1
    Jffs2DataNode dn1(inode_num, f->getLastDataNodeVersion()+1, page_index*LINUX_PAGE_SIZE+offset,
                      node1_count, new_file_size, _gc);
    // add n1 to FS metadata
    Jffs2DataNode *dn1_ptr = f->addDataNode(dn1);
    res = res + _wbuf->writeNode(static_cast<Jffs2DataNode *>(dn1_ptr), MTD_SRC_JFFS2_WRITE_END);

    // node2
    // the preceding write operation should have triggerred a change in current block
    assert(_wbuf->getAddress().getBlock() != old_current_block);
    // set new size if the file grows
    uint32_t node2_count = count - node1_count;
    uint32_t node2_offset = offset + node1_count;
    end_byte_offset = (page_index*LINUX_PAGE_SIZE + node2_offset + node2_count);
    new_file_size = max(end_byte_offset, f->getSize());
    // create dn2
    Jffs2DataNode dn2(inode_num, f->getLastDataNodeVersion()+1, page_index*LINUX_PAGE_SIZE+node2_offset,
                      node2_count, new_file_size, _gc);
    // add n2 to FS metadata
    Jffs2DataNode *dn2_ptr = f->addDataNode(dn2);
    res = res + _wbuf->writeNode(static_cast<Jffs2DataNode *>(dn2_ptr), MTD_SRC_JFFS2_WRITE_END);
  }

  _stats->addJffs2CallTimingInfo(JFFS2_WRITE_END, res.time, res.e_cpu, res.e_mem);

  _gc->gcdTrigger();

  return res;
}

/**
 * Remove system call - file deletion
 */
PpcValF Jffs2::ffsRemove (uint32_t inode_num)
{
  PpcValF res = {0, 0};

  Jffs2File *file = getFile(inode_num);
  if(file == NULL)
    ERROR("JFFS2 file " + nts(inode_num) + " not found for removal");

  // Add a dirent node with #ino 0 to the file, it means it is deleted
  uint32_t last_dirent_node_version = file->getLastDirentNodeVersion();
  assert(last_dirent_node_version > 0);
  uint16_t name_len = file->getNameLen();

  // reserve space
  res = res + reserveSpace(name_len + SIZEOF_JFFS2_RAW_DIRENT);

  Jffs2DirentNode dn(0, last_dirent_node_version+1, name_len, _gc);
  dn.setDeletionDirent();

  //write node
  Jffs2DirentNode *dn_ptr = file->addDirentNode(dn);
  res = res + _wbuf->writeNode(static_cast<Jffs2Node *>(dn_ptr), MTD_SRC_JFFS2_REMOVE);

  _gc->gcdTrigger();

  return res;
}

PpcValF Jffs2::ffsOpen (uint32_t inode_num, bool truncate=false, bool create=false, int name_len=0)
{
  PpcValF res = {0, 0};
  Jffs2File *f;

  if(create && !name_len)
    ERROR("JFFS2 Open with create flag must have name_len parameter > 0");

  if(!create && getFile(inode_num)==NULL)
    ERROR("Jffs2Open (no create flag) called on non existent file (ino=" + nts(inode_num) + ")");

  if(create)
    if((f = getFile(inode_num)) == NULL)
      res = res + ffsCreate(inode_num, name_len);


  /* The first time a file is openned since the FS mount, each one of the file node is read
   * to build the fragment tree
   * we may implement this if we support mount / umount, for now we assume the FS is mounted at the
   * beginning of the simulation and does not contain any data
   */

  if(truncate)
    res = res + ffsTruncate(inode_num, 0);

  //TODO truncate to another size ?

  return res;
}

/**
 * Create system call
 */
PpcValF Jffs2::ffsCreate (uint32_t inode_num, int name_len)
{
  PpcValF res = {0, 0};

  assert(getFile(inode_num) == NULL);

#ifdef VERBOSE
  cout << "[JFFS2] Create ino:" << inode_num << ", name len:" << name_len << endl;
#endif /* VERBOSE */

  // reserve space
    res = res + reserveSpace(name_len + SIZEOF_JFFS2_RAW_DIRENT + SIZEOF_JFFS2_RAW_INODE);

  /* 1. Add a file to the fs metadata */
//  Jffs2File f(inode_num);
  Jffs2File *new_file = new Jffs2File(inode_num);
  _files.push_back(new_file);
  Jffs2File &f = *new_file;

  /* 2. Write a zero sized data node (v1) */
  Jffs2DataNode dn(inode_num, 1, 0, 0, 0, _gc);
  Jffs2DataNode * dn_ptr = f.addDataNode(dn);

  res = res + _wbuf->writeNode(static_cast<Jffs2Node *>(dn_ptr), MTD_SRC_JFFS2_CREATE);

  /* 3. Write dirent */
  Jffs2DirentNode di(inode_num, 1, name_len, _gc);
  Jffs2DirentNode * di_ptr = f.addDirentNode(di);

  res = res + _wbuf->writeNode(static_cast<Jffs2Node *>(di_ptr), MTD_SRC_JFFS2_CREATE);

  _gc->gcdTrigger();

  return res;
}

/**
 * Pop a block from the free list and return its index (for wbuf)
 */
Jffs2Block *Jffs2::getFreeBlock()
{
  return _gc->getFreeBlock();
}

/**
 * JFFS2 does not support page cache writeback. So the only thing to do when syncing is flushing the
 * write buffer
 */
PpcValF Jffs2::ffsSync ()
{
  PpcValF res = {0, 0};
#ifdef VERBOSE
  cout << "[JFFS2] Syncing" << endl;
#endif /* VERBOSE */
  res = res + _wbuf->flush(MTD_SRC_JFFS2_SYNC);

  res = res + _gc->garbageCollectorPass();

  return res;
}

PpcValF Jffs2::ffsTruncate (uint32_t inode_num, uint32_t length)
{
  PpcValF res = {0, 0};
  Jffs2File *f = getFile(inode_num);

  assert(f != NULL);

#ifdef VERBOSE
  cout << "[JFFS2] truncate ino:" << inode_num << ", len:" << length << endl;
#endif /* VERBOSE */

  if(length != 0)
    ERROR("For now jffs2 truncate to length != 0 is not allowed");

  // reserve space
    res = res + reserveSpace(SIZEOF_JFFS2_RAW_INODE);

  uint32_t last_data_node_version = f->getLastDataNodeVersion();
  Jffs2DataNode dn(inode_num, last_data_node_version+1, 0, 0, 0, _gc);
  Jffs2DataNode *dn_ptr = f->addDataNode(dn);		// addDataNode will take care of metadata updating
  res = res + _wbuf->writeNode(static_cast<Jffs2Node *>(dn_ptr), MTD_SRC_JFFS2_TRUNCATE);

  _gc->gcdTrigger();

  return res;
}

PpcValF Jffs2::ffsRename (uint32_t inode, int new_name_len)
{
  PpcValF res = {0, 0};

  Jffs2File *file = getFile(inode);
  if(file == NULL)
    ERROR("JFFS2 file " + nts(inode) + " not found for removal");

  /**
   * Here we write first the dirent with inode 0 then the dirent with the new name.
   * In reality the opposite happen. TODO if it impacts seriously the results we
   * must change the behavior here
   */

  // reserve space
  res = res + reserveSpace(file->getNameLen() + SIZEOF_JFFS2_RAW_DIRENT + SIZEOF_JFFS2_RAW_INODE);

  // First add a dirent node with #ino 0 to the file
  uint32_t last_dirent_node_version = file->getLastDirentNodeVersion();
  assert(last_dirent_node_version > 0);
  uint16_t name_len = file->getNameLen();
  Jffs2DirentNode dn(0, last_dirent_node_version+1, name_len, _gc);
  dn.setRenameDirent();

  //write node
  Jffs2DirentNode *dn_ptr = file->addDirentNode(dn);
  res = res + _wbuf->writeNode(static_cast<Jffs2Node *>(dn_ptr), MTD_SRC_JFFS2_RENAME);

  // Then write the new dirent
  Jffs2DirentNode dn2(inode, last_dirent_node_version+2, new_name_len, _gc);
  Jffs2DirentNode *dn2_ptr = file->addDirentNode(dn2);
  res = res + _wbuf->writeNode(static_cast<Jffs2Node *>(dn2_ptr), MTD_SRC_JFFS2_RENAME);

  _gc->gcdTrigger();

  return res;
}

/**
 * Delete the file from fs metadata
 */
int Jffs2::delFile (Jffs2File* f)
{
  bool ok = false;
  (void)ok;

  for(int i=0; i<(int)_files.size(); i++)
    if(_files[i] == f)
    {
      delete _files[i];
      _files.erase(_files.begin()+i);
      ok = true;
      break;
    }

  assert(ok);

  return 0;
}

/**
 * Trigger gc until the requested amount of free bytes can be available
 */
PpcValF Jffs2::reserveSpace (uint32_t bytes_needed)
{
  PpcValF res = {0, 0};

  uint64_t free_space = _gc->stats_getFreeSpace();

  while(free_space < (bytes_needed+BLOCK_SIZE_BYTE)) /* keep at least 1 free block for GC */
  {
#ifdef VERBOSE
    cout << "[JFFS2] We're low on space, lets GC" << endl;
#endif /* VERBOSE */
    res = res + _gc->garbageCollectorPass();
    free_space = _gc->stats_getFreeSpace();
  }

  return res;
}

void Jffs2::printFileSystem ()
{
  cout << "JFFS2 with " << _files.size() << " files :" << endl;
  for(int i=0; i<(int)_files.size(); i++)
    cout << "  " << *(_files[i]) << endl;
}

PpcValF Jffs2::ffsClose (uint32_t inode)
{
  PpcValF res = {0, 0};
  return res;
}

/**
 * Given an inode, return the pointer to the corresponding Jffs2File
 * object
 * returns null if not found.
 * If bool exist is true, we look if the file was deleted, if it is the case, also return NULL.
 * We can still have a deleted file in FS metadata when its on flash nodes are not erased yet
 */
Jffs2File* Jffs2::getFile (uint32_t inode_num, bool must_exist)
{
  for(int i=0; i<(int)_files.size(); i++)
    if(_files[i]->getInodeNum() == inode_num)
    {
      if(must_exist && _files[i]->_dirent_nodes.back()->getInodeNum() == 0)
	return NULL;
      else
	return _files[i];
    }
  return NULL;
}
