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
#include <utime.h>

#define REPORT_EXCEPTION(e) printf("Exception %s: %s\n\tin: %s\n", typeid(e).name(), e.what(), __PRETTY_FUNCTION__);

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
    DataBase(std::string* db_url);
    ~DataBase();
    void test();

    file_info getByPath(const boost::filesystem::path filename);
    int rename(const boost::filesystem::path oldPath, const boost::filesystem::path newPath);
    int create(boost::filesystem::path path, mode_t mode);
    int remove(boost::filesystem::path filename);
    int truncate(boost::filesystem::path filename, off_t newSize);
    int utime(const boost::filesystem::path filename, struct utimbuf *ubuf);
    bool dirEmpty(file_info dir);
    std::vector<struct file_info> *readdir(file_info *directory);
};

#endif // DATABASE_H
