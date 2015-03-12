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

int DataBase::fstat(std::string path, struct stat* statbuf)
{
  return -1;
}

int DataBase::create(std::string path, mode_t mode)
{

}

int DataBase::unlink(std::string path)
{

}

std::vector<struct file_info> *DataBase::readdir(std::string dirname)
{
  std::vector<struct file_info> * res = new std::vector<struct file_info>();
  sql::PreparedStmt *stmt = db.prepareStmt("SELECT _ID, path, size, mode, uid, gid, mtime, ctime FROM files WHERE path LIKE '?'");
  stmt->bindString(1, dirname+'\%');
  while (stmt->next()) {
    struct file_info fi;
    fi.inode = stmt->getInt(0);
    fi.name = stmt->getString(1).erase(0, dirname.length()+1);
    fi.size = stmt->getInt64(2);
    res->push_back(fi);
  }
  delete stmt;
}


