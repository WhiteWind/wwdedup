#ifndef DATABASE_H
#define DATABASE_H

#include "SqlField.h"
#include "SqlTable.h"
#include "SqlField.h"
#include "SqlDatabase.h"
#include "SqlPreparedStmt.h"

#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include <string>

struct file_info {
  std::string name;
  ino_t inode;
  mode_t mode;
  uid_t uid;
  gid_t gid;
  off_t size;
  unsigned int mtime;
  unsigned int ctime;
};

class DataBase
{
private:
    sql::Database db;
public:
    DataBase(const char* db_url);
    ~DataBase();
    void test();

    int fstat(std::string path, struct stat* statbuf);
    int create(std::string path, mode_t mode);
    int unlink(std::string path);
    std::vector<struct file_info> *readdir(std::string dirname);
};

#endif // DATABASE_H
