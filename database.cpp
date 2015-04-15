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

#include "database.h"
#include "blockscache.h"

using namespace sql;
using namespace boost::filesystem;

Field definition_blockHashes[] =
{
    Field(FIELD_KEY),
    Field("hash", type_blob, flag_not_null | flag_unique),
    Field("use_count", type_int, flag_not_null),
    Field(DEFINITION_END),
};

Field definition_fileBlocks[] =
{
    Field(FIELD_KEY),
    Field("file_id", type_int, flag_not_null),
    Field("offset", type_int, flag_not_null),
    Field("hash", type_blob, flag_not_null),
    Field(DEFINITION_END),
};

Field definition_files[] =
{
    Field(FIELD_KEY),
    Field("path", type_text, flag_not_null),
    Field("depth", type_int, flag_not_null),
    Field("size", type_int, flag_not_null),
    Field("mode", type_int, flag_not_null),
    Field("uid", type_int, flag_not_null),
    Field("gid", type_int, flag_not_null),
    Field("mtime", type_int, flag_not_null),
    Field("ctime", type_int, flag_not_null),
    Field("link_target", type_text),
    Field(DEFINITION_END),
};

DataBase::DataBase(const string *db_url)
{
    assert(db_url);
    db.open(*db_url);

    try {
      //define table object
      Table tbBlockHashes(db.getHandle(), "block_hashes", definition_blockHashes);

      //create new table
      tbBlockHashes.create();

      Table tbFileBlocks(db.getHandle(), "file_blocks", definition_fileBlocks);

      //create new table
      tbFileBlocks.create();

      Table tbFiles(db.getHandle(), "files", definition_files);

      //create new table
      tbFiles.create();

      create("/", S_IFDIR | 0755);
    } catch (sql::Exception &e) {
      printf("At DataBase::DataBase: %s", e.what());
    }
}

void DataBase::test()
{
  Table tbBlockHashes(db.getHandle(), "block_hashes", definition_blockHashes);

  tbBlockHashes.truncate();

  //define new record
  Record record(tbBlockHashes.fields());

  //set record data
  record.setString("hash", "123");
  record.setInteger("_ID", 1);
  record.setInteger("use_count", 1);

  //add 10 records
  //for (int index = 0; index < 10; index++)
          tbBlockHashes.addRecord(&record);

  record.setString("hash", "1234");
  record.setInteger("_ID", 2);
  tbBlockHashes.addRecord(&record);

  //select record to update
  if (Record* record = tbBlockHashes.getRecordByKeyId(2))
  {
          record->setString("hash", "Frank\x01yo\0\x02nigga");
          record->setInteger("use_count", 2);

          tbBlockHashes.updateRecord(record);
  }

  //load all records
  tbBlockHashes.open();

  //list loaded records
  for (int index = 0; index < tbBlockHashes.recordCount(); index++)
          if (Record* record = tbBlockHashes.getRecord(index))
                  sql::log(record->toString());

  sql::log("");
  sql::log("ALL OK");
}

DataBase::~DataBase()
{
  db.close();
}

boost::intrusive_ptr<file_info> DataBase::getByPath(const path filename)
{
  boost::intrusive_ptr<file_info> res = new file_info();
  res->st.st_ino = 0;
  shared_ptr<PreparedStmt> stmt(db.prepareStmt("SELECT _ID, depth, size, mode, uid, gid, mtime, ctime, path FROM files WHERE path = ?"));
  stmt->bindString(1, filename.string());
  if (stmt->next()) {
    res->name = filename;
    res->st.st_ino = stmt->getInt64(0);
    res->depth = stmt->getInt(1);
    res->st.st_size = stmt->getInt64(2);
    res->st.st_mode = stmt->getInt(3);
    res->st.st_uid = stmt->getInt(4);
    res->st.st_gid = stmt->getInt(5);
    res->st.st_mtime = stmt->getInt(6);
    res->st.st_ctime = stmt->getInt(7);
    //printf("Got record: %s, %d, %d, %d %o\n", stmt->getString(8).c_str(), (int)res.st.st_ino, res.depth, (int)res.st.st_size, (int)res.st.st_mode);
  } else {
    res->st.st_ino = 0;
    res->st.st_size = 0;
  }
  return res;
}

int DataBase::rename(const path oldPath, const path newPath)
{
  boost::intrusive_ptr<file_info> oldFile = getByPath(oldPath);
  if (!oldFile->st.st_ino)
    return -ENOENT;
  boost::intrusive_ptr<file_info> destDir = getByPath(newPath.branch_path());
  if (!destDir->st.st_ino)
    return -ENOENT;
  if (!S_ISDIR(destDir->st.st_mode))
    return -ENOTDIR;
  int depthDiff = destDir->depth + 1 - oldFile->depth;
  boost::intrusive_ptr<file_info> newFile = getByPath(newPath);
  if (newFile->st.st_ino) { // DST exists
    if (S_ISDIR(oldFile->st.st_mode)) { // SRC is a directory
      if (S_ISDIR(newFile->st.st_mode) && !dirEmpty(newFile))
        return -ENOTEMPTY;  // DST is non-empty directory
      if (!S_ISDIR(newFile->st.st_mode))
        return -ENOTDIR; // DST is not a directory
      if (newPath.string().find(oldPath.string()) == 0)
        return -EINVAL; // DST is a subdir of SRC
    } else { // SRC is file
      if (S_ISDIR(newFile->st.st_mode))
        return -EISDIR; // DST is a directory
    }
    if (remove(newPath)) {
      return -EBUSY;
    }
  }
  if (S_ISDIR(oldFile->st.st_mode)) {
    shared_ptr<PreparedStmt> stmt(db.prepareStmt("SELECT _ID, path, depth FROM files WHERE path LIKE ?"));
    stmt->bindString(1, oldPath.string() + "/%");
    while (stmt->next()) {
      sqlite3_uint64 id = stmt->getInt64(0);
      string fil = stmt->getString(1);
      fil.replace(0, oldPath.string().length(), newPath.string());
      shared_ptr<PreparedStmt> upd(db.prepareStmt("UPDATE files SET path = ?, depth = depth + ? WHERE _ID = ?"));
      upd->bindString(1, fil);
      upd->bindInt(2, depthDiff);
      upd->bindInt64(3, id);
      upd->next();
    }
  }
  shared_ptr<PreparedStmt> stmt(db.prepareStmt("UPDATE files SET path = ?, depth = ? WHERE _ID = ?"));
  stmt->bindString(1, newPath.string());
  stmt->bindInt(2, destDir->depth + 1);
  stmt->bindInt64(3, oldFile->st.st_ino);
  stmt->next();
  return 0;
}

int DataBase::create(path filename, mode_t mode)
{
    int depth = 0;
    if (filename != "/") {
      boost::intrusive_ptr<file_info> dir = getByPath(filename.branch_path());
      if (!dir->st.st_ino)
        return -ENOENT;
      if ((dir->st.st_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;
      depth = dir->depth + 1;
    }
    boost::intrusive_ptr<file_info> fi = getByPath(filename);
    if (fi->st.st_ino)
      return -EEXIST;
    time_t t = ::time(NULL);
    printf("Creating file %s with mode %o\n", filename.c_str(), (int)mode);
    shared_ptr<PreparedStmt> stmt(db.prepareStmt("INSERT INTO files (path, depth, size, mode, uid, gid, mtime, ctime) VALUES \
        (?, ?, ?, ?, ?, ?, ?, ?)"));
    stmt->bindString(1, filename.c_str());
    stmt->bindInt(2, depth);
    stmt->bindInt(3, 0);
    stmt->bindInt(4, mode);
    stmt->bindInt(5, 0);
    stmt->bindInt(6, 0);
    stmt->bindInt(7, t);
    stmt->bindInt(8, t);
    stmt->next();
    return 0;
}

int DataBase::remove(path filename)
{
  shared_ptr<PreparedStmt> stmt(db.prepareStmt("DELETE FROM files WHERE path = ?"));
  stmt->bindString(1, filename.string());
  stmt->next();
  return 0;
}

int DataBase::truncate(path filename, off_t newSize)
{
  boost::intrusive_ptr<file_info> fi = getByPath(filename);
  if (!fi->st.st_ino)
    return -ENOENT;
  if (!S_ISREG(fi->st.st_mode))
    return -EBADF;
  return ftruncate(fi, newSize);
}

int DataBase::ftruncate(boost::intrusive_ptr<file_info> fi, off_t newSize)
{
  if (newSize < 0)
    return -EINVAL;
  if (fi->st.st_size == newSize)
    return 0;
  db.transactionBegin(tr_exclusive);
  try {
    if (newSize < fi->st.st_size) {
      shared_ptr<PreparedStmt> stmt(db.prepareStmt(
          "SELECT h._ID, b.offset, h.use_count \
          FROM file_blocks b LEFT JOIN block_hashes h \
          ON b.hash = h.hash \
          WHERE b.file_id = ? AND b.offset > ?"));
      stmt->bindInt64(1, fi->st.st_ino);
      stmt->bindInt64(2, newSize >> block_size_bits);
      stringstream buf;
      while (stmt->next()) {
        if (buf.tellp())
          buf << ", ";
        buf << stmt->getInt64(0);
        fi->blocks.erase(stmt->getInt64(1));
        //FIXME: update use_count in BlocksCache
      }
      stmt.reset(db.prepareStmt("UPDATE block_hashes SET use_count = use_count - 1 WHERE _ID IN ("+buf.str()+") AND use_count > 0"));
      stmt->executeUpdate();
      stmt.reset(db.prepareStmt("DELETE FROM file_blocks WHERE file_id = ? AND offset > ?"));
      stmt->bindInt64(1, fi->st.st_ino);
      stmt->bindInt64(2, newSize >> block_size_bits);
      stmt->executeUpdate();
    }
    shared_ptr<PreparedStmt> stmt(db.prepareStmt("UPDATE files SET size = ? WHERE _ID = ?"));
    stmt->bindInt64(1, newSize);
    stmt->bindInt64(2, fi->st.st_ino);
    stmt->next();
  } catch (exception &e) {
    db.transactionRollback();
    throw e;
  }
  fi->st.st_size = newSize;
  db.transactionCommit();
  return 0;
}

int DataBase::utime(const path filename, struct utimbuf *ubuf)
{
  shared_ptr<PreparedStmt> stmt(db.prepareStmt("UPDATE files SET mtime = ? WHERE path = ?"));
  stmt->bindInt(1, ubuf->modtime);
  stmt->bindString(2, filename.string());
  stmt->next();
  return 0;
}

bool DataBase::dirEmpty(boost::intrusive_ptr<file_info> dir)
{
  shared_ptr<PreparedStmt> stmt(db.prepareStmt("SELECT COUNT(*) FROM files WHERE path LIKE ? AND depth = ?"));
  if (*dir->name.string().rbegin() == '/')
    stmt->bindString(1, dir->name.string()+"%");
  else
    stmt->bindString(1, dir->name.string()+"/%");
  stmt->bindInt(2, dir->depth + 1);
  stmt->next();
  int cnt = stmt->getInt(0);
  return !cnt;
}

std::vector<boost::intrusive_ptr<file_info> > DataBase::readdir(boost::intrusive_ptr<file_info> directory)
{
  vector<boost::intrusive_ptr<file_info> > res;
  shared_ptr<PreparedStmt> stmt(db.prepareStmt(
        "SELECT _ID, path, size, mode, uid, gid, mtime, ctime FROM files WHERE path LIKE ? AND depth = ?"));
  if (*directory->name.string().rbegin() == '/')
    stmt->bindString(1, directory->name.string()+"%");
  else
    stmt->bindString(1, directory->name.string()+"/%");
  stmt->bindInt(2, directory->depth + 1);
  while (stmt->next()) {
    boost::intrusive_ptr<file_info> fi = new file_info();
    fi->st.st_ino = stmt->getInt64(0);
    fi->name = stmt->getString(1);
    fi->depth = directory->depth + 1;
    fi->st.st_size = stmt->getInt64(2);
    fi->st.st_mode = stmt->getInt(3);
    fi->st.st_uid = stmt->getInt(4);
    fi->st.st_gid = stmt->getInt(5);
    fi->st.st_mtime = stmt->getInt(6);
    fi->st.st_ctime = stmt->getInt(7);
    //printf("Got record: %s, %d, %d, %d %o\n", fi.name.c_str(), (int)fi.st.st_ino, fi.depth, (int)fi.st.st_size, (int)fi.st.st_mode);
    res.push_back(fi);
  }
  return res;
}

shared_ptr<storage_block> DataBase::getStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t fileBlockNum)
{
  printf("getStorageBlock(f: %ld)\n", fileBlockNum);
  shared_ptr<storage_block> block = make_shared<storage_block>(0);
  if (fileBlockNum * block_size() > finfo->st.st_size)
    return block;

  finfo->blocks.ensure(fileBlockNum, [&](bool bNew, block_info &item, off64_t key)
  {
    unique_lock<mutex> guard(item.lock);
    if (bNew && !item.loaded) {
      shared_ptr<PreparedStmt> stmt(db.prepareStmt(
            "SELECT h._ID, h.hash, h.use_count FROM file_blocks b LEFT JOIN block_hashes h ON b.hash = h.hash WHERE b.file_id = ? AND b.offset = ?"));
      stmt->bindInt64(1, finfo->st.st_ino);
      stmt->bindInt64(2, fileBlockNum);

      if (stmt->next()) {
        item.empty = false;
        item.storageBlockNum = stmt->getInt64(0);
        printf("found block: s: %ld\n", item.storageBlockNum);
        item.hash = make_shared<string>(stmt->getBlob(1));
        block->use_count = stmt->getInt(2);
      } else {
        printf("not found block\n");
        item.empty = true;
        item.storageBlockNum = 0;
        block->use_count = 0;
      }
      item.loaded = true;
    }
    printf("Found block in cache %ld\n", item.storageBlockNum);
    block->storageBlockNum = item.storageBlockNum;
    block->hash = item.hash;
  });

  return block;
}

shared_ptr<storage_block> DataBase::allocateStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t fileBlockNum, shared_ptr<string> hash)
{
  printf("allocateStorageBlock(f: %ld)\n", fileBlockNum);
  db.transactionBegin(tr_exclusive);
  shared_ptr<storage_block> res = make_shared<storage_block>();
  try {
    shared_ptr<PreparedStmt> stmt(db.prepareStmt("SELECT _ID, use_count FROM block_hashes WHERE hash = ?"));
    stmt->bindBlob(1, *hash);
    if (stmt->next()) {
      //present = true;
      res->storageBlockNum = stmt->getInt64(0);
      printf("found existing: s: %ld\n", res->storageBlockNum);
      res->use_count = stmt->getInt(1) + 1;
      res->hash = hash;
      res->loaded = false;
      res->dirty = false;
      stmt.reset(db.prepareStmt("UPDATE block_hashes SET use_count = use_count + 1 WHERE hash = ?"));
      stmt->bindBlob(1, *hash);
      stmt->executeUpdate();
    } else {
      //present = false;
      stmt.reset(db.prepareStmt("INSERT INTO block_hashes (hash, use_count) VALUES(?, 1)"));
      stmt->bindBlob(1, *hash);
      stmt->executeUpdate();
      res->storageBlockNum = db.lastInsertId();
      printf("allocated new: s: %ld\n", res->storageBlockNum);
      res->use_count = 1;
      res->hash = hash;
      res->loaded = false;
      res->dirty = true;
    }
    stmt.reset(db.prepareStmt("INSERT INTO file_blocks (file_id, offset, hash) VALUES(?, ?, ?)"));
    stmt->bindInt64(1, finfo->st.st_ino);
    stmt->bindInt64(2, fileBlockNum);
    stmt->bindBlob(3, *hash);
    stmt->executeUpdate();
    db.transactionCommit();
    finfo->blocks.ensure(fileBlockNum, [res, hash](bool bNew, block_info & item, off64_t key) {
      unique_lock<mutex> guard(item.lock);
      item.empty = false;
      item.loaded = true;
      item.hash = hash;
      item.storageBlockNum = res->storageBlockNum;
    });
    return res;
  } catch (exception &e) {
    db.transactionRollback();
    throw e;
  }
}

void DataBase::releaseStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t fileBlockNum, shared_ptr<storage_block> block)
{
  printf("releaseStorageBlock(f: %ld, s: %ld)\n", fileBlockNum, block->storageBlockNum);
  db.transactionBegin(tr_exclusive);
  try {
    shared_ptr<PreparedStmt> stmt(db.prepareStmt("DELETE FROM file_blocks WHERE file_id = ? AND offset = ?"));
    stmt->bindInt64(1, finfo->st.st_ino);
    stmt->bindInt64(2, fileBlockNum);
    stmt->executeUpdate();

    stmt.reset(db.prepareStmt("UPDATE block_hashes SET use_count = use_count - 1 WHERE hash = ?"));
    stmt->bindBlob(1, *(block->hash));
    stmt->executeUpdate();
    db.transactionCommit();
    block->use_count--;
    finfo->blocks.find(fileBlockNum, [](block_info &item, off64_t key) {
      unique_lock<mutex> guard(item.lock);
      item.hash.reset();
      item.empty = true;
      item.storageBlockNum = 0;
    });
  } catch (exception &e) {
    db.transactionRollback();
    throw e;
  }
}


