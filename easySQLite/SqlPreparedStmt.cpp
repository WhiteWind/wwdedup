/*
 *   This file is part of wwdedup.
 *
 *   wwdedup is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   wwdedup is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "SqlPreparedStmt.h"

namespace sql
{

PreparedStmt::PreparedStmt(sqlite3 *db, std::string sql)
    :_db(db),_columnCount(-2)
{
    int err = sqlite3_prepare_v2(db, sql.c_str(), sql.length() + 1, &_stmt, NULL);
    if (err != SQLITE_OK)
        THROW_EXCEPTION("SqlPreparedStmt::SqlPreparedStmt: error creating prepared statement: " + sql)
}

PreparedStmt::~PreparedStmt()
{
    sqlite3_finalize(_stmt);
}

void PreparedStmt::reset()
{
    sqlite3_reset(_stmt);
    _columnCount = -1;
}

bool PreparedStmt::next()
{
    int res = sqlite3_step(_stmt);
    if (res == SQLITE_ROW) {
        _columnCount = sqlite3_column_count(_stmt);
        return true;
    }

    if (res == SQLITE_DONE) {
        _columnCount = 0;
        return false;
    }

    THROW_EXCEPTION("SqlPreparedStmt::execute: error fetching result")
}

void PreparedStmt::checkColumn(int index)
{
    if (_columnCount < 0)
        THROW_EXCEPTION("Wrong state")
    if (index >= _columnCount)
        THROW_EXCEPTION("Wrong column number")
}

int PreparedStmt::getInt(int index)
{
    return sqlite3_column_int(_stmt, index);
}

sqlite3_int64 PreparedStmt::getInt64(int index)
{
    return sqlite3_column_int64(_stmt, index);
}

double PreparedStmt::getDouble(int index)
{
    return sqlite3_column_double(_stmt, index);
}

string PreparedStmt::getString(int index)
{
    return string((const char*)(sqlite3_column_text(_stmt, index)));
}

string PreparedStmt::getBlob(int index)
{
    const char *ptr = (const char *)sqlite3_column_blob(_stmt, index);
    return std::string(ptr, sqlite3_column_bytes(_stmt, index));
}

bool PreparedStmt::isNull(int index)
{
    return sqlite3_column_type(_stmt, index) == SQLITE_NULL;
}

void PreparedStmt::bindInt(int index, int v)
{
    sqlite3_bind_int(_stmt, index, v);
}

void PreparedStmt::bindInt64(int index, sqlite3_int64 v)
{
    sqlite3_bind_int64(_stmt, index, v);
}

void PreparedStmt::bindDouble(int index, double v)
{
    sqlite3_bind_double(_stmt, index, v);
}

void PreparedStmt::bindString(int index, const string v)
{
    sqlite3_bind_text(_stmt, index, v.c_str(), v.length(), SQLITE_TRANSIENT);
}

void PreparedStmt::bindBlob(int index, const string v)
{
    sqlite3_bind_blob(_stmt, index, v.c_str(), v.length(), SQLITE_TRANSIENT);
}

void PreparedStmt::bindNull(int index)
{
    sqlite3_bind_null(_stmt, index);
}

}
