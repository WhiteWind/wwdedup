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


#ifndef SQLPREPAREDSTMT_H
#define SQLPREPAREDSTMT_H

#include "sqlite3.h"
#include "SqlCommon.h"

namespace sql
{
    class PreparedStmt
    {
    private:
        sqlite3* _db;
        sqlite3_stmt* _stmt;
        int _columnCount;
        void checkColumn(int index);
        string getError();
    public:
        PreparedStmt(sqlite3* db, std::string sql);
        ~PreparedStmt();

        void reset();
        bool next();

        int getInt(int index);
        sqlite3_int64 getInt64(int index);
        double getDouble(int index);
        string getString(int index);
        string getBlob(int index);
        bool isNull(int index);

        void bindInt(int index, int v);
        void bindInt64(int index, sqlite3_int64 v);
        void bindDouble(int index, double v);
        void bindString(int index, const string v);
        void bindBlob(int index, const string v);
        void bindNull(int index);

        void executeUpdate(bool retry = false);
    };

}
#endif // SQLPREPAREDSTMT_H
