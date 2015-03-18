#include "database.h"

using namespace sql;

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
    Field(DEFINITION_END),
};

DataBase::DataBase(const char *db_url)
{
    db.open(db_url);
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

file_info DataBase::getByPath(const std::string path)
{
  file_info res;
  res.st.st_ino = 0;
  sql::PreparedStmt *stmt = db.prepareStmt("SELECT _ID, depth, size, mode, uid, gid, mtime, ctime, path FROM files WHERE path = ?");
  stmt->bindString(1, path);
  if (stmt->next()) {
    res.name = path;
    res.st.st_ino = stmt->getInt64(0);
    res.depth = stmt->getInt(1);
    res.st.st_size = stmt->getInt64(2);
    res.st.st_mode = stmt->getInt(3);
    res.st.st_uid = stmt->getInt(4);
    res.st.st_gid = stmt->getInt(5);
    res.st.st_mtime = stmt->getInt(6);
    res.st.st_ctime = stmt->getInt(7);
    //printf("Got record: %s, %d, %d, %d %o\n", stmt->getString(8).c_str(), (int)res.st.st_ino, res.depth, (int)res.st.st_size, (int)res.st.st_mode);
  } else {
    res.st.st_ino = 0;
    res.st.st_size = 0;
  }
  delete stmt;
  return res;
}

int DataBase::create(std::string path, mode_t mode)
{
  try {
    int depth = 0;
    if (path != "/") {
      char * path_c = strdup(path.c_str());
      file_info dir = getByPath(dirname(path_c));
      free(path_c);
      if (!dir.st.st_ino)
        return -ENOENT;
      if ((dir.st.st_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;
      depth = dir.depth + 1;
    }
    file_info fi = getByPath(path);
    if (fi.st.st_ino)
      return -EEXIST;
    time_t t = ::time(NULL);
    printf("Creating file %s with mode %o\n", path.c_str(), (int)mode);
    sql::PreparedStmt *stmt = db.prepareStmt("INSERT INTO files (path, depth, size, mode, uid, gid, mtime, ctime) VALUES \
        (?, ?, ?, ?, ?, ?, ?, ?)");
    stmt->bindString(1, path.c_str());
    stmt->bindInt(2, depth);
    stmt->bindInt(3, 0);
    stmt->bindInt(4, mode);
    stmt->bindInt(5, 0);
    stmt->bindInt(6, 0);
    stmt->bindInt(7, t);
    stmt->bindInt(8, t);
    stmt->next();
    return 0;
  } catch(Exception e) {
    printf("Exception %s\n", e.msg().c_str());
    return -EIO;
  }
}

int DataBase::unlink(std::string path)
{

}

std::vector<struct file_info> *DataBase::readdir(file_info *directory)
{
  std::vector<struct file_info> * res = new std::vector<struct file_info>();
  sql::PreparedStmt *stmt = db.prepareStmt(
        "SELECT _ID, path, size, mode, uid, gid, mtime, ctime FROM files WHERE path LIKE ? AND depth = ?");
  stmt->bindString(1, directory->name+'\%');
  stmt->bindInt(2, directory->depth + 1);
  while (stmt->next()) {
    struct file_info fi;
    fi.st.st_ino = stmt->getInt64(0);
    fi.name = stmt->getString(1);
    fi.depth = directory->depth + 1;
    fi.st.st_size = stmt->getInt64(2);
    fi.st.st_mode = stmt->getInt(3);
    fi.st.st_uid = stmt->getInt(4);
    fi.st.st_gid = stmt->getInt(5);
    fi.st.st_mtime = stmt->getInt(6);
    fi.st.st_ctime = stmt->getInt(7);
    res->push_back(fi);
  }
  delete stmt;
  return res;
}


