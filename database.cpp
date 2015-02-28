#include "database.h"

using namespace sql;

Field definition_blockHashes[] =
{
    Field(FIELD_KEY),
    Field("hash", type_text, flag_not_null | flag_unique),
    Field("use_count", type_int, flag_not_null),
    Field(DEFINITION_END),
};

Field definition_fileBlocks[] =
{
    Field(FIELD_KEY),
    Field("file_id", type_int, flag_not_null),
    Field("offset", type_int, flag_not_null),
    Field("hash", type_text, flag_not_null),
    Field(DEFINITION_END),
};

Field definition_files[] =
{
    Field(FIELD_KEY),
    Field("path", type_text, flag_not_null),
    Field("size", type_int, flag_not_null),
    Field("mode", type_int, flag_not_null),
    Field("mtime", type_int, flag_not_null),
    Field("ctime", type_int, flag_not_null),
    Field(DEFINITION_END),
};

DataBase::DataBase()
{
    db.open("test.db");
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
          record->setString("hash", "Frank");
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

}

int DataBase::stat(std::string path, struct stat* statbuf)
{
  return -ENOENT;
}

int DataBase::create(std::string path, mode_t mode)
{

}

