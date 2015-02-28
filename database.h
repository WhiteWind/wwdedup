#ifndef DATABASE_H
#define DATABASE_H

#include "SqlField.h"
#include "SqlTable.h"
#include "SqlField.h"
#include "SqlDatabase.h"

class DataBase
{
private:
    sql::Database db;
public:
    DataBase();
    ~DataBase();
    void test();
};

#endif // DATABASE_H
