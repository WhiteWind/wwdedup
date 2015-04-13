#include "SqlDatabase.h"
#include "SqlRecordSet.h"
#include <time.h>


namespace sql
{

Database::Database(void)
{
	_db = NULL;
	_result_open = SQLITE_ERROR;

	close();

        tzset();
}

Database::~Database(void)
{
	close();
}

sqlite3* Database::getHandle()
{
	return _db;
}

string Database::errMsg()
{
  return _err_msg;
}

void Database::close()
{
	if (_db)
	{
		sqlite3_close(_db);
		_db = NULL;
		_err_msg.clear();
		_result_open = SQLITE_ERROR;
	}
}

bool Database::isOpen()
{
	return (_result_open == SQLITE_OK);
}

bool Database::open(string filename)
{
  close();

  sqlite3_enable_shared_cache(false);
  _result_open = sqlite3_open(filename.c_str(), &_db);

  if (isOpen())
  {
    PreparedStmt stmt(_db, "PRAGMA journal_mode=WAL");
    while (true)
      try {
        stmt.executeUpdate();
        return true;
      } catch (DatabaseLockedException &e) {}
  } else {
    _err_msg = sqlite3_errmsg(_db);
  }

  THROW_EXCEPTION("Database::open: " + errMsg())

  return false;
}

PreparedStmt* Database::prepareStmt(const string sql)
{
  return new PreparedStmt(_db, sql);
}

sqlite_int64 Database::lastInsertId()
{
  return sqlite3_last_insert_rowid(_db);
}

bool Database::transactionBegin(transaction_mode _mode)
{
	RecordSet rs(_db);

  string q = "BEGIN";
  if (_mode == tr_immediate)
    q += " IMMEDIATE";
  if (_mode == tr_exclusive)
    q += " EXCLUSIVE";
  q += " TRANSACTION";
  if (rs.query(q))
		return true;

	return false;
}

bool Database::transactionCommit()
{
	RecordSet rs(_db);

	if (rs.query("COMMIT TRANSACTION"))
		return true;

	return false;
}

bool Database::transactionRollback()
{
	RecordSet rs(_db);

	if (rs.query("ROLLBACK TRANSACTION"))
		return true;

	return false;
}


//sql eof
};
