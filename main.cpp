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

#include <cds/init.h>       // for cds::Initialize and cds::Terminate
#include <cds/gc/dhp.h>      // for cds::HP (Hazard Pointer) SMR
#include <fuse.h>
#include <stdio.h>

#include "wrap.h"
#include "SqlCommon.h"
#include "database.h"
#include "blockscache.h"

struct fuse_operations examplefs_oper;

void log_fun(void* priv, int errcode, const char* msg)
{
  printf("SQLITE Error %d: %s\n", errcode, msg);
}

int main(int argc, char *argv[]) {
  cds::Initialize();

  cds::gc::DHP gc;
  cds::threading::Manager::attachThread();

  int i, fuse_stat = 1;

  examplefs_oper.getattr = wrap_getattr;
  examplefs_oper.readlink = wrap_readlink;
  examplefs_oper.getdir = NULL;
  examplefs_oper.mknod = wrap_mknod;
  examplefs_oper.mkdir = wrap_mkdir;
  examplefs_oper.unlink = wrap_unlink;
  examplefs_oper.rmdir = wrap_rmdir;
  examplefs_oper.symlink = wrap_symlink;
  examplefs_oper.rename = wrap_rename;
  examplefs_oper.link = wrap_link;
  examplefs_oper.chmod = wrap_chmod;
  examplefs_oper.chown = wrap_chown;
  examplefs_oper.truncate = wrap_truncate;
  examplefs_oper.utime = wrap_utime;
  examplefs_oper.open = wrap_open;
  examplefs_oper.create = wrap_create;
  examplefs_oper.read = wrap_read;
  examplefs_oper.write = wrap_write;
  examplefs_oper.statfs = wrap_statfs;
  examplefs_oper.flush = wrap_flush;
  examplefs_oper.release = wrap_release;
  examplefs_oper.fsync = wrap_fsync;
  examplefs_oper.setxattr = NULL;
  examplefs_oper.getxattr = NULL;
  examplefs_oper.listxattr = NULL;
  examplefs_oper.removexattr = NULL;
//  examplefs_oper.setxattr = wrap_setxattr;
//  examplefs_oper.getxattr = wrap_getxattr;
//  examplefs_oper.listxattr = wrap_listxattr;
//  examplefs_oper.removexattr = wrap_removexattr;
  examplefs_oper.opendir = wrap_opendir;
  examplefs_oper.readdir = wrap_readdir;
  examplefs_oper.releasedir = wrap_releasedir;
  examplefs_oper.fsyncdir = wrap_fsyncdir;
  examplefs_oper.init = wrap_init;
  examplefs_oper.ftruncate = wrap_ftruncate;

  printf("%lX: mounting file system...\n", pthread_self());

  for(i = 1; i < argc && (argv[i][0] == '-'); i++) {
    if(i == argc) {
      return (-1);
    }
  }

  std::string dbUrl = argv[i];
  sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
  sqlite3_config(SQLITE_CONFIG_LOG, log_fun, NULL);
  if (!sqlite3_threadsafe())
    printf("!!ERROR: SQLite is not threadsafe!!\n");

  try
  {
    for(; i < argc; i++) {
      argv[i] = argv[i+1];
    }
    argc--;

    {
      DataBase db(&dbUrl);
    }

    BlocksCache::start(&dbUrl);

    fuse_stat = fuse_main(argc, argv, &examplefs_oper, &dbUrl);

    printf("fuse_main returned %d\n", fuse_stat);
//    delete db;
  } catch (sql::Exception e) {
    printf("ERROR: %s\r\n", e.msg().c_str());
  }

  BlocksCache::stop();

  cds::Terminate();
  return fuse_stat;
}


