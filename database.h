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
#include <libgen.h>
#include <errno.h>
#include <time.h>
#include <cstring>

struct file_info {
  std::string name;
  int depth;
  struct stat st;
};

class DataBase
{
private:
    sql::Database db;
public:
    DataBase(const char* db_url);
    ~DataBase();
    void test();

    file_info getByPath(const std::string path);
    int create(std::string path, mode_t mode);
    int unlink(std::string path);
    std::vector<struct file_info> *readdir(file_info *directory);
};

#endif // DATABASE_H
