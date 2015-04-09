#ifndef BLOCKSCACHE_H
#define BLOCKSCACHE_H

#include <string>
#include <cds/gc/dhp.h>
#include <cds/container/skip_list_set_dhp.h>
#include "database.h"

class BlocksCache
{
private:
  static BlocksCache *_instance;
  DataBase *db;
  cds::container::SkipListSet<cds::gc::DHP, shared_ptr<block_info> > _blocks;
  void run(const string *db_url);
  volatile int terminated;
  thread thr;
  BlocksCache(const string *db_url);
  ~BlocksCache();
  void _setBlock(shared_ptr<block_info> block);
  shared_ptr<block_info> _getBlock(shared_ptr<file_info> finfo, off64_t offset);
  void _sync();
public:
  static void start(const string *db_url) { if (!_instance) _instance = new BlocksCache(db_url); }
  static void stop() { delete _instance; _instance = nullptr; }
  static void setBlock(shared_ptr<block_info> block) { _instance->_setBlock(block); }
  static shared_ptr<block_info> getBlock(shared_ptr<file_info> finfo, off64_t offset) { return _instance->_getBlock(finfo, offset); }
  static void sync() { _instance->_sync(); }
};

#endif // BLOCKSCACHE_H
