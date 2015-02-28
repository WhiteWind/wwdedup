#ifndef DATABASE_H
#define DATABASE_H

#include "SqlField.h"
#include "SqlTable.h"
#include "SqlField.h"
#include "SqlDatabase.h"

#include <errno.h>

class DataBase
{
private:
    sql::Database db;
public:
    DataBase();
    ~DataBase();
    void test();

    int stat(std::string path, struct stat* statbuf);
    int create(std::string path, mode_t mode);
};

#endif // DATABASE_H
