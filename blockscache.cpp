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
  _storage = open(storagepath.c_str(), O_CREAT | O_DSYNC | O_RDWR);
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

void BlocksCache::_write(shared_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
{

}

off64_t BlocksCache::_read(shared_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
{
  if (size + offset > finfo->st.st_size)
    size = finfo->st.st_size - offset;
  off64_t startBlockNum = offset >> block_size_bits;
  off64_t endBlockNum = (offset + size) >> block_size_bits;
  off64_t factSize = 0;
  shared_ptr<block_info> firstBlock = _getBlock(finfo, startBlockNum);
  int startOffset = offset - (startBlockNum << block_size_bits);
  int chunkSize = min(size, (off64_t)(block_size() - startOffset));
  memcpy(buf, firstBlock->data->data() + startOffset, chunkSize);
  factSize += chunkSize;
  while (factSize < size) {
    assert(startBlockNum <= endBlockNum);
    shared_ptr<block_info> block = _getBlock(finfo, ++startBlockNum);
    chunkSize = min((ptrdiff_t)block_size(), size - factSize);
    memcpy(buf + factSize, block->data->data(), chunkSize);
    factSize += chunkSize;
  }
  return factSize;
}

void BlocksCache::_erase(shared_ptr<file_info> finfo, off64_t size, off64_t offset)
{

}

shared_ptr<block_info> BlocksCache::_getBlock(shared_ptr<file_info> finfo, off64_t blockNum)
{
  string hash;
  off64_t storageBlockNum = db->getStorageBlockNum(finfo.get(), blockNum, hash);
  if (!storageBlockNum)
    return make_shared<block_info>(finfo, blockNum);
  auto gp = _blocks.get(make_shared<block_info>(storageBlockNum));
  shared_ptr<block_info> block;
  if (gp)
    block = *gp;
  else
    block = make_shared<block_info>(finfo, blockNum);
  if (!block->storageBlockNum) {
    block->storageBlockNum = db->getStorageBlockNum(finfo.get(), blockNum);
    block->data = make_shared<vector<unsigned char> >(block_size(), '\0');
    if (block->storageBlockNum) {
      read(_storage, block->data->data(), block_size());
    }
  }
  return block;
}

void BlocksCache::_sync()
{

}

