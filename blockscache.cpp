#include "blockscache.h"

BlocksCache *BlocksCache::_instance = nullptr;

void BlocksCache::run(const string *db_url)
{
  cds::gc::DHP::thread_gc gc;
  db = new DataBase(db_url);
  while (!terminated) {
    try {

    } catch (exception &e) {
      REPORT_EXCEPTION(e)
    }
  }
  delete db;
}

BlocksCache::BlocksCache(const string *db_url)
  : terminated(false), thr(&BlocksCache::run, this, db_url)
{

}

BlocksCache::~BlocksCache()
{
  terminated = true;
  thr.join();
}

void BlocksCache::_setBlock(shared_ptr<block_info> block)
{
  _blocks.insert(block);
}

shared_ptr<block_info> BlocksCache::_getBlock(shared_ptr<file_info> finfo, off64_t offset)
{
  auto gp = _blocks.get(make_shared<block_info>(new block_info(finfo, offset, nullptr, nullptr)));
  shared_ptr<block_info> block;
  if (gp)
    block = *gp;
  else
    block = make_shared<block_info>(finfo, offset, nullptr, nullptr);
  if (!block->storage_offset) {
    block->storage_offset = db->getStorageOffset(finfo.get(), offset);
    //TODO: read from storage
  }
  return block;
}

void BlocksCache::_sync()
{

}

