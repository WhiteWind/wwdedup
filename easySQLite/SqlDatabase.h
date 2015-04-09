//
// Copyright (C) 2010 Piotr Zagawa
//
// Released under BSD License
//

#pragma once

#include "sqlite3.h"
#include "SqlCommon.h"
#include "SqlPreparedStmt.h"


namespace sql
{

class Database
{
private:
	sqlite3* _db;
	string _err_msg;
	int _result_open;

public:
	Database(void);
	~Database(void);

public:
	sqlite3* getHandle();
	string errMsg();
	bool open(string filename);
	void close();
	bool isOpen();

        PreparedStmt* prepareStmt(const string sql);
        sqlite_int64 lastInsertId();

public:
        bool transactionBegin(transaction_mode _mode = tr_deferred);
	bool transactionCommit();
	bool transactionRollback();

};


//sql eof
};
