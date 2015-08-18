/*
 *   This file is part of wwdedup.
 *
 *   wwdedup is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   wwdedup is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "blockscache.h"

static boost::thread_specific_ptr<BlocksCache> _instance;

int _storage;
std::mutex writerMutex;
std::condition_variable writerFlag;
std::atomic_bool writeTriggered;
cds::container::SkipListSet<cds::gc::DHP, shared_ptr<storage_block>,
    typename cds::container::skip_list::make_traits<
        cds::opt::compare<storage_block_comparator>,
        cds::opt::item_counter<cds::atomicity::item_counter>
>::type
  > *_blocks;

void BlocksCache::run(const string *db_url)
{
  cds::gc::DHP::thread_gc gc;
  cds::threading::Manager::attachThread();
  //FIXME: race condition
  _blocks = new cds::container::SkipListSet<cds::gc::DHP, shared_ptr<storage_block>,
      typename cds::container::skip_list::make_traits<
          cds::opt::compare<storage_block_comparator>,
          cds::opt::item_counter<cds::atomicity::item_counter> >::type>();
  unique_lock<mutex> lck(writerMutex);
  time_t mintime;
  bool repeat = false;
  while (!terminated) {
    if (!repeat) {
      writerFlag.wait_for(lck, chrono::seconds(5), [this] {return !!writeTriggered;});
      writeTriggered = false;
    }
    repeat = false;
    //cout << "Writer woke up" << endl;
    mintime = time(nullptr);
    try {
      for (auto iter = _blocks->begin(); iter != _blocks->end(); ++iter) {
        shared_ptr<storage_block> block = *iter;
        unique_lock<mutex> guard(block->lock);
        mintime = min(mintime, block->atime);
//        cout << "Found block " << *block << endl;
        if (block->dirty) {
          printf("Writing block %lX (%x%x%x%x)\n", block->storageBlockNum,
                 block->data[0], block->data[1], block->data[2], block->data[3]);
          pwrite(_storage, block->data.data(), block_size, block->storageBlockNum << block_size_bits);
          block->dirty = false;
        }
      }
      cout << "Cached blocks count: " << _blocks->size() << endl;
      if (_blocks->size() > 500) {
        vector<shared_ptr<storage_block> > toDelete;
        for (auto iter = _blocks->begin(); iter != _blocks->end(); ++iter) {
          if (toDelete.size() > 63)
            break;
          shared_ptr<storage_block> block = *iter;
          if (block->atime < mintime+10) {
            unique_lock<mutex> guard(block->lock);
            if (block->dirty) {
              pwrite(_storage, block->data.data(), block_size, block->storageBlockNum << block_size_bits);
              block->dirty = false;
            }
            toDelete.push_back(block);
          }
        }
        for (auto iter : toDelete) {
          cout << "Releasing storage block " << *iter << endl;
          _blocks->erase(iter);
        }
        if (_blocks->size() > 500)
          repeat = true;
      }
      this_thread::sleep_for(chrono::milliseconds(1));
    } catch (exception &e) {
      REPORT_EXCEPTION(e)
    }
  }
  cds::threading::Manager::detachThread();
}

BlocksCache::BlocksCache(const string *db_url, int _block_size_bits)
  : db(nullptr), terminated(false), block_size(1 << _block_size_bits), block_size_bits(_block_size_bits)
{
  writeTriggered = false;
  boost::filesystem::path storagepath = boost::filesystem::path(*db_url).branch_path();
  storagepath /= "storage.blk";
  _storage = open(storagepath.c_str(), O_CREAT | O_DSYNC | O_RDWR, 0644);
  if (_storage < 0)
    throw system_error(errno, system_category(), "Could not create storage file " + storagepath.string());
  off64_t stSize = lseek(_storage, 0, SEEK_END);
  if (stSize == 0) {
    lseek(_storage, 0, SEEK_SET);
    char signature[block_size];
    memcpy(signature, "WWDEDUP1", 8);
    ((int32_t*)signature)[2] = block_size;
    if (write(_storage, signature, block_size) != block_size)
      throw system_error(errno, system_category(), "Can not write storage file " + storagepath.string());
  } else {
    lseek(_storage, 0, SEEK_SET);
    char signature[block_size];
    if (read(_storage, signature, block_size) != block_size)
      throw system_error(errno, system_category(), "Can not read storage file "+storagepath.string());
    if (memcmp("WWDEDUP1", signature, 8))
      throw runtime_error("Wrong or corrupt storage file "+storagepath.string());
    if (((int32_t*)signature)[2] != block_size)
      throw runtime_error("Wrong block size in storage file "+storagepath.string());
  }
  thr = thread(&BlocksCache::run, this, db_url);
}

BlocksCache::~BlocksCache()
{
  terminated = true;
  if (thr.joinable())
    thr.join();
  close(_storage);
}

void BlocksCache::_populateStorageBlock(shared_ptr<storage_block> block)
{
  if (!block->data.empty() || !block->storageBlockNum)
    return;
  block->data.resize(block_size);
  lseek(_storage, block->storageBlockNum << block_size_bits, SEEK_SET);
  if (read(_storage, block->data.data(), block_size) != block_size)
    throw system_error(errno, system_category(), "Error reading block from storage");
}

void BlocksCache::start(const string *db_url, int block_size_bits) {
  if (!_instance.get())
    _instance.reset(new BlocksCache(db_url, block_size_bits));
}

void BlocksCache::stop()
{
  _instance.reset();
}

BlocksCache *BlocksCache::getThreadInstance(DataBase *_db, int _block_size_bits)
{
  if (!_instance.get())
    _instance.reset(new BlocksCache(_db, _block_size_bits));
  if (!_instance->db)
    _instance->db = _db;
  return _instance.get();
}

void BlocksCache::_writeBlock(boost::intrusive_ptr<file_info> finfo, vector<unsigned char> &data, block_info* fblock)
{
  printf("_writeBlock(%ld)\n", fblock->fileBlockNum);
  shared_ptr<string> hash = make_shared<string>(16, '\0');
  MurmurHash3_x64_128(data.data(), block_size, HASH_SEED, (void*)hash->c_str());

  //switch
  //case present: find block, use_count++
  //case replaced: find block, move(data), dirty=true, use_count = 1, set hash
  //case new: create block, move(data), dirty=true, use_count = 1, set hash
  if (db->replaceStorageBlock(finfo, fblock, hash)) {
    // block with this hash already present
    shared_ptr<storage_block> tmp = make_shared<storage_block>(fblock->storageBlockNum);
    _blocks->find(tmp, [&](shared_ptr<storage_block> item, shared_ptr<storage_block>){
      item->use_count++;
    });
  } else {
    // need to write block. If old block is released, database sets it's use_count to 0
    _blocks->ensure(make_shared<storage_block>(fblock->storageBlockNum), [&](bool bNew, shared_ptr<storage_block> &item, const shared_ptr<storage_block>){
      unique_lock<mutex> guard(item->lock);
      item->data = move(data);
      item->hash = hash;
      item->use_count = 1;
      item->dirty = true;
      item->loaded = true;
      fblock->storageBlock = item;
    });
  }
}

void BlocksCache::_writeBlockWrapper(boost::intrusive_ptr<file_info> finfo, const char *curPtr, off64_t curBlockNum, int chunkSize, int startOffset)
{
  block_info *fileBlock = _getLockedFileBlock(finfo, curBlockNum);
  vector<unsigned char> data;
  shared_ptr<storage_block> firstBlock = fileBlock->storageBlock.lock();
  if (firstBlock) {
    unique_lock<mutex> guard(firstBlock->lock);
    firstBlock->access();
    if (firstBlock->storageBlockNum) {
      if (chunkSize < block_size) {
        _populateStorageBlock(firstBlock);
        data = firstBlock->data;
      }
      //db->releaseStorageBlock(finfo, curBlockNum, firstBlock);
    }
  }
  if (data.empty())
    data.resize(block_size);

  memcpy(data.data() + startOffset, curPtr, chunkSize);
  _writeBlock(finfo, data, fileBlock);
  fileBlock->lock.unlock();
}

off64_t BlocksCache::writeBuf(boost::intrusive_ptr<file_info> finfo, const char *buf, off64_t size, off64_t offset)
{
  if (size + offset > finfo->st.st_size)
    db->ftruncate(finfo, size+offset);
  off64_t startBlockNum = offset >> block_size_bits;
  int startOffset = offset - (startBlockNum << block_size_bits);
  off64_t endBlockNum = (offset + size - 1) >> block_size_bits;
  off64_t factSize = 0;
  int chunkSize = min(size, (off64_t)(block_size - startOffset));
  off64_t curBlockNum = startBlockNum;

  _writeBlockWrapper(finfo, buf, curBlockNum, chunkSize, startOffset);
  factSize += chunkSize;

  while (factSize < size) {
    assert(curBlockNum <= endBlockNum);
    chunkSize = min((off64_t)block_size, size - factSize);
    _writeBlockWrapper(finfo, buf + factSize, ++curBlockNum, chunkSize);
    factSize += chunkSize;
  }
  return factSize;
}

off64_t BlocksCache::readBuf(boost::intrusive_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
{
  if (size + offset > finfo->st.st_size)
    size = finfo->st.st_size - offset;
  off64_t startBlockNum = offset >> block_size_bits;
  off64_t endBlockNum = (offset + size) >> block_size_bits;
  off64_t factSize = 0;
  int startOffset = offset - (startBlockNum << block_size_bits);
  int chunkSize = min(size, (off64_t)(block_size - startOffset));
  {
    shared_ptr<storage_block> firstBlock = _getStorageBlock(finfo, startBlockNum);
    unique_lock<mutex> guard(firstBlock->lock);
    if (firstBlock->storageBlockNum) {
      _populateStorageBlock(firstBlock);
      memcpy(buf, firstBlock->data.data() + startOffset, chunkSize);
    } else
      memset(buf, 0, chunkSize);
  }
  factSize += chunkSize;
  while (factSize < size) {
    assert(startBlockNum <= endBlockNum);
    chunkSize = min((ptrdiff_t)block_size, size - factSize);
    {
      shared_ptr<storage_block> block = _getStorageBlock(finfo, ++startBlockNum);
      unique_lock<mutex> guard(block->lock);
      if (block->storageBlockNum) {
        _populateStorageBlock(block);
        memcpy(buf + factSize, block->data.data(), chunkSize);
      } else
        memset(buf + factSize, 0, chunkSize);
    }
    factSize += chunkSize;
  }
  return factSize;
}

int BlocksCache::truncate(boost::intrusive_ptr<file_info> finfo, off64_t newSize)
{
  if (newSize < 0)
    return -EINVAL;
  if (finfo->st.st_size <= newSize) {
    return db->ftruncate(finfo, newSize);
  }
  off64_t startBlock = newSize ? ((newSize - 1) >> block_size_bits) + 1 : 0;
  off64_t endBlock = finfo->st.st_size ? ((finfo->st.st_size - 1) >> block_size_bits) + 1 : 1;
  for (off64_t curBlock = startBlock; curBlock < endBlock; curBlock++) {
    off64_t storageBlockNum = db->releaseStorageBlock(finfo, curBlock);
    if (storageBlockNum) {
      auto key = make_shared<storage_block>(storageBlockNum);
      _blocks->find(key,
                    [](shared_ptr<storage_block> &sBlock, shared_ptr<storage_block>) {
                      sBlock->use_count--;
                    });
    }
//    block_info * fblock = _getLockedFileBlock(finfo, curBlock);
//    try {
//      shared_ptr<storage_block> sblock = fblock->storageBlock.lock();
//      fblock->lock.unlock();
//      if (sblock) {
//        unique_lock<mutex> guard(sblock->lock);
//        db->releaseStorageBlock(finfo, curBlock, sblock);
//      }
//    } catch (exception &e) {
//      fblock->lock.unlock();
//      throw e;
//    }
  }
  return db->ftruncate(finfo, newSize);
}

shared_ptr<storage_block> BlocksCache::_getStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t blockNum)
{
  printf("_getBlock(%ld)\n", blockNum);

  block_info *fblock = _getLockedFileBlock(finfo, blockNum);
  if (!fblock)
    return make_shared<storage_block>();
  shared_ptr<storage_block> res = fblock->storageBlock.lock();
  fblock->lock.unlock();
  if (!res)
    return make_shared<storage_block>();
  else {
    res->access();
    return res;
  }
}

block_info *BlocksCache::_getLockedFileBlock(boost::intrusive_ptr<file_info> finfo, off64_t blockNum)
{
  block_info *fblock;
  int use_count = 0;

  finfo->blocks.ensure(blockNum, [&](bool bNew, block_info &item, off64_t key){
    item.lock.lock();
    if (bNew && !item.loaded) {
      db->loadFileBlock(finfo, item, use_count);
      item.loaded = true;
    }
    //FIXME: can item be deleted by another thread?
    fblock = &item;
  });

  if (fblock->empty) {
    return fblock;
  }

  //FIXME: storageBlock may be deleted by another thread
  if (!fblock->storageBlock.expired())
    return fblock;

  _blocks->ensure(make_shared<storage_block>(fblock->storageBlockNum),
                 [&](bool bNew, shared_ptr<storage_block> &item, shared_ptr<storage_block>) {
    unique_lock<mutex> guard(item->lock);
    if (bNew && !item->loaded) {
      item->use_count = use_count;
      item->hash = fblock->hash;
      cout << "Storage block placed: " << *item << endl;
      item->loaded = true;
    } else {
      assert(!use_count || (item->use_count == use_count));
    }
    fblock->storageBlock = item;
  });
  return fblock;
}

void BlocksCache::sync()
{
  unique_lock<mutex> lck(writerMutex);
  cout << "Sync" << endl;
  writeTriggered = true;
  writerFlag.notify_one();
}
