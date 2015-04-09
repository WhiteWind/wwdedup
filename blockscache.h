#ifndef BLOCKSCACHE_H
#define BLOCKSCACHE_H

#include <string>
#include <exception>
#include <cds/gc/dhp.h>
#include <cds/container/skip_list_set_dhp.h>
#include <boost/filesystem.hpp>
#include "database.h"

struct storage_block {
  off64_t storageBlockNum;
  mutex lock;
  shared_ptr<string> hash;
  shared_ptr<vector<unsigned char> > data;
  bool dirty;
  storage_block(off64_t num): storageBlockNum(num) {}
  bool operator < (const storage_block &other ) const
    {
      return storageBlockNum < other.storageBlockNum;
    }

  void calc_hash()
    {
      hash = make_shared<string>(128, 0);
      MurmurHash3_x64_128(data->data(), block_size(), HASH_SEED, (void*)hash->c_str());
    }
};

class BlocksCache
{
private:
  static BlocksCache *_instance;
  int _storage;
  DataBase *db;
  cds::container::SkipListSet<cds::gc::DHP, shared_ptr<storage_block> > _blocks;
  void run(const string *db_url);
  volatile int terminated;
  thread thr;
  BlocksCache(const string *db_url);
  ~BlocksCache();
  void _write(file_info *finfo, char *buf, off64_t size, off64_t offset);
  off64_t _read(file_info *finfo, char *buf, off64_t size, off64_t offset);
  void _erase(file_info * finfo, off64_t size, off64_t offset);
  void _sync();

  shared_ptr<storage_block> _getBlock(file_info *finfo, off64_t blockNum);
public:
  static void start(const string *db_url) { if (!_instance) _instance = new BlocksCache(db_url); }
  static void stop() { delete _instance; _instance = nullptr; }

  static void writeBuf(file_info *finfo, char *buf, off64_t size, off64_t offset)
      { _instance->_write(finfo, buf, size, offset); }
  static off64_t readBuf(file_info *finfo, char *buf, off64_t size, off64_t offset)
      { return _instance->_read(finfo, buf, size, offset); }
  static void erase(file_info *finfo, off64_t size, off64_t offset)
      { _instance->_erase(finfo, size, offset); }
  static void sync() { _instance->_sync(); }
};

#endif // BLOCKSCACHE_H
