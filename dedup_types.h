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
#ifndef DEDUP_TYPES_H
#define DEDUP_TYPES_H

#include <iostream>
#include <cds/gc/dhp.h>
#include <cds/container/skip_list_set_dhp.h>
#include <boost/filesystem.hpp>
#include <fcntl.h>
#include "MurmurHash3.h"

#define HASH_SEED 0xad64fac3

using namespace std;


#define block_size_bits 16

static int block_size()
{
  return 1 << block_size_bits;
}

struct storage_block {
  off64_t storageBlockNum;
  mutex lock;
  int use_count;
  shared_ptr<string> hash;
  vector<unsigned char> data;
  bool dirty;
  bool loaded;

  storage_block() :storageBlockNum(0), use_count(0), hash(), data(), dirty(false), loaded(false)
    { printf("storage_block()\n"); }
  storage_block(off64_t num)
    : storageBlockNum(num), use_count(0), hash(), data(), dirty(false), loaded(false)
    { printf("storage_block(%ld)\n", num); }
  storage_block(const storage_block &other)
    : storageBlockNum(other.storageBlockNum), use_count(other.use_count),
      hash(other.hash), data(other.data), dirty(other.dirty), loaded(other.loaded)
  {}
  ~storage_block() { printf("~storage_block(%ld)\n", storageBlockNum); }

  void calc_hash()
    {
      hash = make_shared<string>(128, 0);
      MurmurHash3_x64_128(data.data(), block_size(), HASH_SEED, (void*)hash->c_str());
    }
};

struct storage_block_comparator{
  int operator() (const shared_ptr<storage_block> &a, const shared_ptr<storage_block> &b);
};

ostream& operator << (ostream& stream, const storage_block & block);

struct block_info {
  off64_t fileBlockNum;
  off64_t storageBlockNum;
  shared_ptr<string> hash;
  mutex lock;
  bool loaded;
  bool empty;

  block_info(off64_t _fileBlockNum)
    : fileBlockNum(_fileBlockNum), storageBlockNum(0), hash(), loaded(false), empty(true) {}
  block_info(off64_t _fileBlockNum, off64_t _storageBlockNum, string _hash)
    : fileBlockNum(_fileBlockNum), storageBlockNum(_storageBlockNum), hash(make_shared<string>(_hash)), loaded(true), empty(false) {}
  block_info(off64_t _fileBlockNum, bool _empty)
    : fileBlockNum(_fileBlockNum), storageBlockNum(0), hash(), loaded(true), empty(_empty) {}

  bool operator < (const block_info &other) const
    {
      return fileBlockNum < other.fileBlockNum;
    }
};

struct file_info {
  atomic_int refcount;
  boost::filesystem::path name;
  int depth;
  struct stat st;
  cds::container::SkipListSet<cds::gc::DHP, block_info> blocks;

  file_info(): refcount(0), depth(0) { printf("file_info()\n"); }
  ~file_info() { printf("~file_info(%s)\n", name.c_str()); }
};

void intrusive_ptr_add_ref(file_info* p);
void intrusive_ptr_release(file_info* p);

#endif // DEDUP_TYPES_H
