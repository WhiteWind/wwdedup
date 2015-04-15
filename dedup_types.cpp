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
#include "dedup_types.h"

int storage_block_comparator::operator ()(const shared_ptr<storage_block> &a, const shared_ptr<storage_block> &b)
{
  if (!a && b)
    return -1;
  if (a && !b)
    return 1;
  if (!a && !b)
    return 0;
  if (a->storageBlockNum < b->storageBlockNum)
    return -1;
  if (a->storageBlockNum > b->storageBlockNum)
    return 1;
  return 0;
}

ostream& operator << (ostream& stream, const storage_block & block)
{
  stream << "storage_block(" << &block << "){" << block.storageBlockNum << ", u: " << block.use_count;
  if (block.loaded)
    stream << " loaded ";
  if (block.dirty)
    stream << " dirty ";
  stream << ' ';
  stream << std::hex;
  if (block.hash)
    for (auto c = block.hash->cbegin(); c != block.hash->cend(); ++c)
      stream << +((unsigned char)(*c));
  stream << std::dec;
  return stream << "}";
}

void intrusive_ptr_add_ref(file_info* p)
{
  //printf("add_ref(file_info) [%d, %s]\n", p->refcount.load(), p->name.c_str());
  ++p->refcount;
}

void intrusive_ptr_release(file_info* p)
{
  //printf("release(file_info) [%d, %s]\n", p->refcount.load(), p->name.c_str());
  if (!--p->refcount) {
    delete p;
  }
}
