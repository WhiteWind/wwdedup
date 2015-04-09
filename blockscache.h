#ifndef BLOCKSCACHE_H
#define BLOCKSCACHE_H

#include <string>
#include <exception>
#include <cds/gc/dhp.h>
#include <cds/container/skip_list_set_dhp.h>
#include <boost/filesystem.hpp>
#include "database.h"

class BlocksCache
{
private:
  static BlocksCache *_instance;
  int _storage;
  DataBase *db;
  cds::container::SkipListSet<cds::gc::DHP, shared_ptr<block_info> > _blocks;
  void run(const string *db_url);
  volatile int terminated;
  thread thr;
  BlocksCache(const string *db_url);
  ~BlocksCache();
  void _write(shared_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset);
  off64_t _read(shared_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset);
  void _erase(shared_ptr<file_info> finfo, off64_t size, off64_t offset);
  void _sync();

  shared_ptr<block_info> _getBlock(shared_ptr<file_info> finfo, off64_t blockNum);
public:
  static void start(const string *db_url) { if (!_instance) _instance = new BlocksCache(db_url); }
  static void stop() { delete _instance; _instance = nullptr; }

  static void writeBuf(shared_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
      { _instance->_write(finfo, buf, size, offset); }
  static off64_t readBuf(shared_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
      { return _instance->_read(finfo, buf, size, offset); }
  static void erase(shared_ptr<file_info> finfo, off64_t size, off64_t offset)
      { _instance->_erase(finfo, size, offset); }
  static void sync() { _instance->_sync(); }
};

#endif // BLOCKSCACHE_H
