#include "blockscache.h"

BlocksCache *BlocksCache::_instance = nullptr;

void BlocksCache::run(const string *db_url)
{
  cds::gc::DHP::thread_gc gc;
  db = new DataBase(db_url);
  while (!terminated) {
    try {
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
    throw runtime_error("Could not create storage file " + storagepath.string());
  off64_t stSize = lseek(_storage, 0, SEEK_END);
  if (stSize == 0) {
    lseek(_storage, 0, SEEK_SET);
    char *signature = (char*)malloc(block_size());
    memcpy(signature, "WWDEDUP1", 8);
    write(_storage, signature, block_size());
  }
  thr = thread(&BlocksCache::run, this, db_url);
}

BlocksCache::~BlocksCache()
{
  terminated = true;
  thr.join();
  close(_storage);
}

void BlocksCache::_write(file_info *finfo, char *buf, off64_t size, off64_t offset)
{

}

off64_t BlocksCache::_read(file_info *finfo, char *buf, off64_t size, off64_t offset)
{
  if (size + offset > finfo->st.st_size)
    size = finfo->st.st_size - offset;
  off64_t startBlockNum = offset >> block_size_bits;
  off64_t endBlockNum = (offset + size) >> block_size_bits;
  off64_t factSize = 0;
  shared_ptr<storage_block> firstBlock = _getBlock(finfo, startBlockNum);
  int startOffset = offset - (startBlockNum << block_size_bits);
  int chunkSize = min(size, (off64_t)(block_size() - startOffset));
  memcpy(buf, firstBlock->data->data() + startOffset, chunkSize);
  factSize += chunkSize;
  while (factSize < size) {
    assert(startBlockNum <= endBlockNum);
    shared_ptr<storage_block> block = _getBlock(finfo, ++startBlockNum);
    chunkSize = min((ptrdiff_t)block_size(), size - factSize);
    memcpy(buf + factSize, block->data->data(), chunkSize);
    factSize += chunkSize;
  }
  return factSize;
}

void BlocksCache::_erase(file_info *finfo, off64_t size, off64_t offset)
{

}

shared_ptr<storage_block> BlocksCache::_getBlock(file_info *finfo, off64_t blockNum)
{
  string hash;
  off64_t storageBlockNum = db->getStorageBlockNum(finfo, blockNum, &hash);
  if (!storageBlockNum) {
     shared_ptr<storage_block> block = make_shared<storage_block>(0);
     block->data = make_shared<vector<unsigned char> >(block_size(), '\0');
     return block;
  }

  shared_ptr<storage_block> block;
  _blocks.ensure(make_shared<storage_block>(storageBlockNum),
                 [this, &block](bool bNew, shared_ptr<storage_block> &item, shared_ptr<storage_block> key) {
    unique_lock<mutex>(item->lock);
    if (bNew) {
      item->data = make_shared<vector<unsigned char> >(block_size(), '\0');
      read(_storage, item->data->data(), block_size());
    }
    block = item;
  });

  return block;
}

void BlocksCache::_sync()
{

}

