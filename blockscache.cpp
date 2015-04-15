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

BlocksCache *BlocksCache::_instance = nullptr;

void BlocksCache::run(const string *db_url)
{
  cds::gc::DHP::thread_gc gc;
  db = new DataBase(db_url);
  while (!terminated) {
    try {
      for (auto iter = _blocks.begin(); iter != _blocks.end(); ++iter) {
        shared_ptr<storage_block> block = *iter;
        unique_lock<mutex> guard(block->lock);
//        cout << "Found block " << *block << endl;
        if (block->dirty) {
          printf("Writing block %lX (%x%x%x%x)\n", block->storageBlockNum,
                 block->data[0], block->data[1], block->data[2], block->data[3]);
          lseek(_storage, block->storageBlockNum << block_size_bits, SEEK_SET);
          write(_storage, block->data.data(), block_size());
          block->dirty = false;
        }
      }
      this_thread::yield();
    } catch (exception &e) {
      REPORT_EXCEPTION(e)
    }
  }
  delete db;
}

BlocksCache::BlocksCache(const string *db_url)
  : terminated(false)
{
  boost::filesystem::path storagepath = boost::filesystem::path(*db_url).branch_path();
  storagepath /= "storage.blk";
  _storage = open(storagepath.c_str(), O_CREAT | O_DSYNC | O_RDWR, 0644);
  if (_storage < 0)
    throw system_error(errno, system_category(), "Could not create storage file " + storagepath.string());
  off64_t stSize = lseek(_storage, 0, SEEK_END);
  if (stSize == 0) {
    lseek(_storage, 0, SEEK_SET);
    char signature[block_size()];
    memcpy(signature, "WWDEDUP1", 8);
    ((int32_t*)signature)[2] = block_size();
    if (write(_storage, signature, block_size()) != block_size())
      throw system_error(errno, system_category(), "Can not write storage file " + storagepath.string());
  } else {
    lseek(_storage, 0, SEEK_SET);
    char signature[block_size()];
    if (read(_storage, signature, block_size()) != block_size())
      throw system_error(errno, system_category(), "Can not read storage file "+storagepath.string());
    if (memcmp("WWDEDUP1", signature, 8))
      throw runtime_error("Wrong or corrupt storage file "+storagepath.string());
    if (((int32_t*)signature)[2] != block_size())
      throw runtime_error("Wrong block size in storage file "+storagepath.string());
  }
  thr = thread(&BlocksCache::run, this, db_url);
}

BlocksCache::~BlocksCache()
{
  terminated = true;
  thr.join();
  close(_storage);
}

void BlocksCache::_writeBlock(boost::intrusive_ptr<file_info> finfo, vector<unsigned char> &data, off64_t fileBlockNum)
{
  printf("_writeBlock(%ld)\n", fileBlockNum);
  shared_ptr<string> hash = make_shared<string>(16, '\0');
  MurmurHash3_x64_128(data.data(), block_size(), HASH_SEED, (void*)hash->c_str());
  shared_ptr<storage_block> newBlock = db->allocateStorageBlock(finfo, fileBlockNum, hash);
  newBlock->data = move(data);
  _blocks.ensure(newBlock, [](bool bNew, shared_ptr<storage_block> &item, shared_ptr<storage_block> key){
    unique_lock<mutex> guard(item->lock);
    if (bNew)
      cout << "Storage block placed: " << *item << endl;
    if (bNew && item != key) {
      item->data = key->data;
      item->dirty = key->dirty;
      item->hash = key->hash;
      item->use_count = key->use_count;
    } else {
      if (item->hash->compare(*key->hash)) {
        assert(item->use_count == 0);
        item->dirty = true;
        item->hash = key->hash;
      } else if (!item->loaded)
        item->data = key->data;
      item->use_count = key->use_count;
    }
    item->loaded = true;
  });
}

off64_t BlocksCache::_write(boost::intrusive_ptr<file_info> finfo, const char *buf, off64_t size, off64_t offset)
{
  if (size + offset > finfo->st.st_size)
    db->ftruncate(finfo, size+offset);
  off64_t startBlockNum = offset >> block_size_bits;
  int startOffset = offset - (startBlockNum << block_size_bits);
  off64_t endBlockNum = (offset + size - 1) >> block_size_bits;
  int endBlockSize = (offset + size) - (endBlockNum << block_size_bits);
  off64_t factSize = 0;
  int chunkSize = min(size, (off64_t)(block_size() - startOffset));
  {
    shared_ptr<storage_block> firstBlock = _getBlock(finfo, startBlockNum);
    vector<unsigned char> data = vector<unsigned char>(firstBlock->data);
    if (firstBlock->storageBlockNum)
      db->releaseStorageBlock(finfo, startBlockNum, firstBlock);
    memcpy(data.data() + startOffset, buf + factSize, chunkSize);
    _writeBlock(finfo, data, startBlockNum);
  }
  factSize += chunkSize;

  off64_t curBlockNum = startBlockNum;
  while (factSize < size) {
    assert(curBlockNum <= endBlockNum);
    {
      //FIXME: no need to read from disk if replacing whole block
      shared_ptr<storage_block> oldBlock = _getBlock(finfo, ++curBlockNum);
      vector<unsigned char> data = vector<unsigned char>(oldBlock->data);
      if (oldBlock->storageBlockNum)
        db->releaseStorageBlock(finfo, curBlockNum, oldBlock);
      chunkSize = min((off64_t)block_size(), size - factSize);
      memcpy(data.data(), buf + factSize, chunkSize);
      _writeBlock(finfo, data, curBlockNum);
    }
    factSize += chunkSize;
  }
  return factSize;
}

off64_t BlocksCache::_read(boost::intrusive_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
{
  if (size + offset > finfo->st.st_size)
    size = finfo->st.st_size - offset;
  off64_t startBlockNum = offset >> block_size_bits;
  off64_t endBlockNum = (offset + size) >> block_size_bits;
  off64_t factSize = 0;
  shared_ptr<storage_block> firstBlock = _getBlock(finfo, startBlockNum);
  int startOffset = offset - (startBlockNum << block_size_bits);
  int chunkSize = min(size, (off64_t)(block_size() - startOffset));
  memcpy(buf, firstBlock->data.data() + startOffset, chunkSize);
  factSize += chunkSize;
  while (factSize < size) {
    assert(startBlockNum <= endBlockNum);
    shared_ptr<storage_block> block = _getBlock(finfo, ++startBlockNum);
    chunkSize = min((ptrdiff_t)block_size(), size - factSize);
    memcpy(buf + factSize, block->data.data(), chunkSize);
    factSize += chunkSize;
  }
  return factSize;
}

void BlocksCache::_erase(boost::intrusive_ptr<file_info> finfo, off64_t size, off64_t offset)
{

}

shared_ptr<storage_block> BlocksCache::_getBlock(boost::intrusive_ptr<file_info> finfo, off64_t blockNum)
{
  printf("_getBlock(%ld)\n", blockNum);
  string hash;
  //off64_t storageBlockNum ;
  shared_ptr<storage_block> block = db->getStorageBlock(finfo, blockNum);
  cout << "DB returned: " << *block.get() << endl;
  if (!block->storageBlockNum) {
    //FIXME: no need to allocate zeroes
    block->data = vector<unsigned char> (block_size(), '\0');
    return block;
  }

  shared_ptr<storage_block> res;
  _blocks.ensure(block,
                 [this, &res](bool bNew, shared_ptr<storage_block> &item, shared_ptr<storage_block> key) {
    unique_lock<mutex> guard(item->lock);
    if (bNew && !item->loaded) {
      assert(key->use_count > 0);
      cout << "Storage block placed: " << *item << endl;
      item->use_count = key->use_count;
      item->data = vector<unsigned char>(block_size(), '\0');
      lseek(_storage, item->storageBlockNum << block_size_bits, SEEK_SET);
      if (read(_storage, item->data.data(), block_size()) != block_size())
        throw system_error(errno, system_category(), "Error reading block from storage");
    }
    item->loaded = true;
    res = item;
  });

  return res;
}

void BlocksCache::_sync()
{

}

