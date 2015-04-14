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
#ifndef BLOCKSCACHE_H
#define BLOCKSCACHE_H

#include <string>
#include <exception>
#include <system_error>
#include "database.h"
#include "dedup_types.h"


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
  off64_t _write(boost::intrusive_ptr<file_info> finfo, const char *buf, off64_t size, off64_t offset);
  off64_t _read(boost::intrusive_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset);
  void _erase(boost::intrusive_ptr<file_info> finfo, off64_t size, off64_t offset);
  void _sync();

  shared_ptr<storage_block> _getBlock(boost::intrusive_ptr<file_info> finfo, off64_t blockNum);
public:
  static void start(const string *db_url) { if (!_instance) _instance = new BlocksCache(db_url); }
  static void stop() { delete _instance; _instance = nullptr; }

  static off64_t writeBuf(boost::intrusive_ptr<file_info> finfo, const char *buf, off64_t size, off64_t offset)
      { return _instance->_write(finfo, buf, size, offset); }
  static off64_t readBuf(boost::intrusive_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset)
      { return _instance->_read(finfo, buf, size, offset); }
  static void erase(boost::intrusive_ptr<file_info> finfo, off64_t size, off64_t offset)
      { _instance->_erase(finfo, size, offset); }
  static void sync() { _instance->_sync(); }
  void _writeBlock(boost::intrusive_ptr<file_info> finfo, vector<unsigned char> &data, off64_t fileBlockNum);
};

#endif // BLOCKSCACHE_H
