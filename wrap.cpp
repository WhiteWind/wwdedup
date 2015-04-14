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


#include "wrap.h"
#include "dedupfs.h"

int wrap_getattr(const char *path, struct stat *statbuf) {
  try {
    return DedupFS::Instance()->Getattr(path, statbuf);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_readlink(const char *path, char *link, size_t size) {
  try {
    return DedupFS::Instance()->Readlink(path, link, size);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_mknod(const char *path, mode_t mode, dev_t dev) {
  try {
        return DedupFS::Instance()->Mknod(path, mode, dev);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_mkdir(const char *path, mode_t mode) {
  try {
        return DedupFS::Instance()->Mkdir(path, mode);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_unlink(const char *path) {
  try {
        return DedupFS::Instance()->Unlink(path);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_rmdir(const char *path) {
  try {
        return DedupFS::Instance()->Rmdir(path);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_symlink(const char *path, const char *link) {
  try {
        return DedupFS::Instance()->Symlink(path, link);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_rename(const char *path, const char *newpath) {
  try {
        return DedupFS::Instance()->Rename(path, newpath);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_link(const char *path, const char *newpath) {
  try {
        return DedupFS::Instance()->Link(path, newpath);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_chmod(const char *path, mode_t mode) {
  try {
        return DedupFS::Instance()->Chmod(path, mode);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_chown(const char *path, uid_t uid, gid_t gid) {
  try {
        return DedupFS::Instance()->Chown(path, uid, gid);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_truncate(const char *path, off_t newSize) {
  try {
        return DedupFS::Instance()->Truncate(path, newSize);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_utime(const char *path, struct utimbuf *ubuf) {
  try {
        return DedupFS::Instance()->Utime(path, ubuf);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_open(const char *path, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Open(path, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_create(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Create(path, mode, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Read(path, buf, size, offset, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Write(path, buf, size, offset, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_statfs(const char *path, struct statvfs *statInfo) {
  try {
        return DedupFS::Instance()->Statfs(path, statInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_flush(const char *path, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Flush(path, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_release(const char *path, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Release(path, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
  try {
        return DedupFS::Instance()->Fsync(path, datasync, fi);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
  try {
        return DedupFS::Instance()->Setxattr(path, name, value, size, flags);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_getxattr(const char *path, const char *name, char *value, size_t size) {
  try {
        return DedupFS::Instance()->Getxattr(path, name, value, size);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_listxattr(const char *path, char *list, size_t size) {
  try {
        return DedupFS::Instance()->Listxattr(path, list, size);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_removexattr(const char *path, const char *name) {
  try {
        return DedupFS::Instance()->Removexattr(path, name);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_opendir(const char *path, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Opendir(path, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Readdir(path, buf, filler, offset, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_releasedir(const char *path, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Releasedir(path, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

int wrap_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
  try {
        return DedupFS::Instance()->Fsyncdir(path, datasync, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}

void *wrap_init(struct fuse_conn_info *conn) {
  try {
    return DedupFS::Instance()->Init(conn);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return nullptr;
  }
}

int wrap_ftruncate(const char *path, off_t newSize, fuse_file_info *fileInfo)
{
  try {
    return DedupFS::Instance()->Ftruncate(path, newSize, fileInfo);
  } catch (std::exception &e) {
    REPORT_EXCEPTION(e)
    return -EIO;
  }
}
