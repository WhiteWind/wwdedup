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
#ifndef examples_hh
#define examples_hh

#define FUSE_USE_VERSION 26

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <pthread.h>
#include <atomic>
#include <cds/gc/dhp.h>
#include <cds/container/skip_list_map_dhp.h>
#include "database.h"

class DedupFS {
private: 

  DataBase *db;

public:
  static DedupFS *Instance();

  DedupFS();
  ~DedupFS();

  int Getattr(const char *path, struct stat *statbuf);
  int Readlink(const char *path, char *link, size_t size);
  int Mknod(const char *path, mode_t mode, dev_t dev);
  int Mkdir(const char *path, mode_t mode);
  int Unlink(const char *path);
  int Rmdir(const char *path);
  int Symlink(const char *path, const char *link);
  int Rename(const char *path, const char *newpath);
  int Link(const char *path, const char *newpath);
  int Chmod(const char *path, mode_t mode);
  int Chown(const char *path, uid_t uid, gid_t gid);
  int Truncate(const char *path, off_t newSize);
  int Utime(const char *path, struct utimbuf *ubuf);
  int Open(const char *path, struct fuse_file_info *fileInfo);
  int Create(const char *path, mode_t mode, struct fuse_file_info *fileInfo);
  int Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
  int Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
  int Statfs(const char *path, struct statvfs *statInfo);
  int Flush(const char *path, struct fuse_file_info *fileInfo);
  int Release(const char *path, struct fuse_file_info *fileInfo);
  int Fsync(const char *path, int datasync, struct fuse_file_info *fi);
  int Setxattr(const char *path, const char *name, const char *value, size_t size, int flags);
  int Getxattr(const char *path, const char *name, char *value, size_t size);
  int Listxattr(const char *path, char *list, size_t size);
  int Removexattr(const char *path, const char *name);
  int Opendir(const char *path, struct fuse_file_info *fileInfo);
  int Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
  int Releasedir(const char *path, struct fuse_file_info *fileInfo);
  int Fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo);
  void *Init(struct fuse_conn_info *conn);
  int Truncate(const char *path, off_t offset, struct fuse_file_info *fileInfo);
};



#endif //examples_hh
