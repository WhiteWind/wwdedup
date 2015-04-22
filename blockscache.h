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

#include "pre.h"
#include "MurmurHash3.h"
#include "database.h"
#include "dedup_types.h"


class BlocksCache
{
private:
  DataBase *db;
  void run(const string *db_url);
  volatile int terminated;
  thread thr;
  BlocksCache(const string *db_url);
  BlocksCache(DataBase *_db): db(_db) {}

  shared_ptr<storage_block> _getStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t blockNum);
  block_info *_getLockedFileBlock(boost::intrusive_ptr<file_info> finfo, off64_t blockNum);
  void _writeBlock(boost::intrusive_ptr<file_info> finfo, vector<unsigned char> &data, block_info *fblock);
  void _populateStorageBlock(shared_ptr<storage_block> block);
  void _writeBlockWrapper(boost::intrusive_ptr<file_info> finfo, const char *curPtr, off64_t curBlockNum, int chunkSize, int startOffset = 0);
public:
  ~BlocksCache();
  static void start(const string *db_url);
  static BlocksCache *getThreadInstance(DataBase *_db);
  static void stop();

  off64_t writeBuf(boost::intrusive_ptr<file_info> finfo, const char *buf, off64_t size, off64_t offset);
  off64_t readBuf(boost::intrusive_ptr<file_info> finfo, char *buf, off64_t size, off64_t offset);
  int truncate(boost::intrusive_ptr<file_info> finfo, off64_t newSize);
  void sync();
};

#endif // BLOCKSCACHE_H
