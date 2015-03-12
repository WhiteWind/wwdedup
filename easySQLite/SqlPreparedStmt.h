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
    };

}
#endif // SQLPREPAREDSTMT_H
