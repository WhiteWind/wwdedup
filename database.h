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

#ifndef DATABASE_H
#define DATABASE_H

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <libgen.h>
#include <errno.h>
#include <time.h>
#include <cstring>
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/smart_ptr.hpp>
#include <utime.h>

#include "SqlField.h"
#include "SqlTable.h"
#include "SqlField.h"
#include "SqlDatabase.h"
#include "SqlPreparedStmt.h"
#include "dedup_types.h"

#define REPORT_EXCEPTION(e) printf("Exception %s: %s\n\tin: %s\n", typeid(e).name(), e.what(), __PRETTY_FUNCTION__);

using namespace std;



class DataBase
{
private:
    sql::Database db;
public:
    DataBase(const std::string* db_url);
    ~DataBase();
    void test();

    boost::intrusive_ptr<file_info> getByPath(const boost::filesystem::path filename);
    int rename(const boost::filesystem::path oldPath, const boost::filesystem::path newPath);
    int create(boost::filesystem::path path, mode_t mode);
    int remove(boost::filesystem::path filename);
    int truncate(boost::filesystem::path filename, off_t newSize);
    int ftruncate(boost::intrusive_ptr<file_info> fi, off_t newSize);
    int utime(const boost::filesystem::path filename, struct utimbuf *ubuf);
    bool dirEmpty(boost::intrusive_ptr<file_info> dir);
    std::vector<boost::intrusive_ptr<file_info> > readdir(boost::intrusive_ptr<file_info> directory);
    shared_ptr<storage_block> getStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t fileBlockNum);
    shared_ptr<storage_block> allocateStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t fileBlockNum, shared_ptr<string> hash);
    void releaseStorageBlock(boost::intrusive_ptr<file_info> finfo, off64_t fileBlockNum, shared_ptr<storage_block> block);
};

#endif // DATABASE_H
