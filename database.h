#ifndef DATABASE_H
#define DATABASE_H

#include "SqlField.h"
#include "SqlTable.h"
#include "SqlField.h"
#include "SqlDatabase.h"
#include "SqlPreparedStmt.h"

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <libgen.h>
#include <errno.h>
#include <time.h>
#include <cstring>
#include <boost/filesystem/path.hpp>

struct file_info {
  boost::filesystem::path name;
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

    file_info getByPath(const boost::filesystem::path filename);
    int rename(const boost::filesystem::path oldPath, const boost::filesystem::path newPath);
    int create(boost::filesystem::path path, mode_t mode);
    int remove(boost::filesystem::path filename);
    bool dirEmpty(file_info dir);
    std::vector<struct file_info> *readdir(file_info *directory);
};

#endif // DATABASE_H
