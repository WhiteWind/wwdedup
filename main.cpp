#include "wrap.h"
#include "SqlCommon.h"
#include "database.h"

#include <fuse.h>
#include <stdio.h>

struct fuse_operations examplefs_oper;

int main(int argc, char *argv[]) {
  int i, fuse_stat;

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

  printf("mounting file system...\n");

  for(i = 1; i < argc && (argv[i][0] == '-'); i++) {
    if(i == argc) {
      return (-1);
    }
  }

  try
  {
    DataBase *db = new DataBase(argv[i]);
    //db->test();

    set_database(db);

    for(; i < argc; i++) {
      argv[i] = argv[i+1];
    }
    argc--;

    fuse_stat = fuse_main(argc, argv, &examplefs_oper, NULL);

    printf("fuse_main returned %d\n", fuse_stat);
    delete db;
  } catch (sql::Exception e) {
          printf("ERROR: %s\r\n", e.msg().c_str());
  }

  return fuse_stat;
}


