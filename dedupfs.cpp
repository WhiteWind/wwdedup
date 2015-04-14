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

#include <boost/thread/tss.hpp>
#include "dedupfs.h"

static boost::thread_specific_ptr<DedupFS> _instance;
static std::atomic_uint th_count(0);

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

DedupFS* DedupFS::Instance() {
  if(!_instance.get()) {
    _instance.reset(new DedupFS());
    if ( !cds::threading::Manager::isThreadAttached() )
      cds::threading::Manager::attachThread();
  }
  printf("%lX: ", pthread_self());
  return _instance.get();
}

DedupFS::DedupFS() {
  printf("%lX/%d: DedupFS::DedupFS()\n", pthread_self(), ++th_count);
  db = new DataBase(static_cast<std::string*>(fuse_get_context()->private_data));
}

DedupFS::~DedupFS() {
  printf("%lX/%d: DedupFS::~DedupFS()\n", pthread_self(), --th_count);
  delete db;
}

int DedupFS::Getattr(const char *path, struct stat *statbuf) {
  boost::intrusive_ptr<file_info> fi = db->getByPath(path);
  if (fi->st.st_ino) {
    *statbuf = fi->st;
    printf("getattr: SUCCESS %s %d %o\n", path, (int)fi->st.st_ino, (int)fi->st.st_mode);
    return 0;
  } else {
    printf("getattr: ENOENT %s\n", path);
    return -ENOENT;
  }
}

int DedupFS::Readlink(const char *path, char *link, size_t size) {
        printf("readlink(path=%s, link=%s, size=%d)\n", path, link, (int)size);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(readlink(fullPath, link, size));
  return -ENOSYS;
}

int DedupFS::Mknod(const char *path, mode_t mode, dev_t dev) {
        printf("mknod(path=%s, mode=%o)\n", path, mode);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
	
//	//handles creating FIFOs, regular files, etc...
//	return RETURN_ERRNO(mknod(fullPath, mode, dev));
  return -ENOSYS;
}

int DedupFS::Mkdir(const char *path, mode_t mode) {
  printf("**mkdir(path=%s, mode=%d)\n", path, (int)mode);
  return db->create(path, mode | S_IFDIR);
}

int DedupFS::Unlink(const char *path) {
  printf("unlink(path=%s)\n", path);
  boost::intrusive_ptr<file_info> fi = db->getByPath(path);
  if (!fi->st.st_ino)
    return -ENOENT;
  if (S_ISDIR(fi->st.st_mode))
    return -EISDIR;
  return db->remove(path);
}

int DedupFS::Rmdir(const char *path) {
  printf("rmkdir(path=%s\n)", path);
  boost::intrusive_ptr<file_info> fi = db->getByPath(path);
  if (!S_ISDIR(fi->st.st_mode))
    return -ENOTDIR;
  if (!db->dirEmpty(fi))
    return -ENOTEMPTY;
  return db->remove(path);
}

int DedupFS::Symlink(const char *path, const char *link) {
        printf("symlink(path=%s, link=%s)\n", path, link);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(symlink(fullPath, link));
  return -ENOSYS;
}

int DedupFS::Rename(const char *path, const char *newpath) {
  printf("rename(path=%s, newPath=%s)\n", path, newpath);
  return db->rename(path, newpath);
}

int DedupFS::Link(const char *path, const char *newpath) {
        printf("link(path=%s, newPath=%s)\n", path, newpath);
//	char fullPath[PATH_MAX];
//	char fullNewPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	AbsPath(fullNewPath, newpath);
//	return RETURN_ERRNO(link(fullPath, fullNewPath));
  return -ENOSYS;
}

int DedupFS::Chmod(const char *path, mode_t mode) {
        printf("chmod(path=%s, mode=%d)\n", path, mode);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(chmod(fullPath, mode));
  return -ENOSYS;
}

int DedupFS::Chown(const char *path, uid_t uid, gid_t gid) {
        printf("chown(path=%s, uid=%d, gid=%d)\n", path, (int)uid, (int)gid);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(chown(fullPath, uid, gid));
  return -ENOSYS;
}

int DedupFS::Truncate(const char *path, off_t newSize) {
  printf("truncate(path=%s, newSize=%d\n", path, (int)newSize);
  return db->truncate(path, newSize);
}

int DedupFS::Utime(const char *path, struct utimbuf *ubuf) {
  printf("utime(path=%s)\n", path);
  return db->utime(path, ubuf);
}

int DedupFS::Open(const char *path, struct fuse_file_info *fileInfo) {
  printf("open(path=%s)\n", path);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	fileInfo->fh = open(fullPath, fileInfo->flags);
  boost::intrusive_ptr<file_info> fi = db->getByPath(path);
  if (!fi->st.st_ino)
    return -ENOENT;
  if (!S_ISREG(fi->st.st_mode))
    return -EIO;
  fileInfo->fh = (uint64_t)fi.detach();
  return 0;
//  return -ENOSYS;
}

int DedupFS::Create(const char *path, mode_t mode, struct fuse_file_info *fileInfo)
{
  printf("create(path=%s, mode=%o)\n", path, (int)mode);
  int res = db->create(path, mode);
  if (res >= 0) { // FIXME: return file_info, not int
    boost::intrusive_ptr<file_info> fi = db->getByPath(path);
    fileInfo->fh = (uint64_t)fi.detach();
  } else
    fileInfo->fh = 0;
  return res;
}

int DedupFS::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
  printf("read(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
  return BlocksCache::readBuf((file_info*)fileInfo->fh, buf, size, offset);
  //return -ENOSYS;
}

int DedupFS::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
  printf("write(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
  return BlocksCache::writeBuf((file_info*)fileInfo->fh, buf, size, offset);
}

int DedupFS::Statfs(const char *path, struct statvfs *statInfo) {
        printf("statfs(path=%s)\n", path);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(statvfs(fullPath, statInfo));
  return -ENOSYS;
}

int DedupFS::Flush(const char *path, struct fuse_file_info *fileInfo) {
  printf("flush(path=%s)\n", path);
//	//noop because we don't maintain our own buffers
  return 0;
}

int DedupFS::Release(const char *path, struct fuse_file_info *fileInfo) {
  printf("release(path=%s)\n", path);
  intrusive_ptr_release((file_info*)fileInfo->fh);
  return 0;
}

int DedupFS::Fsync(const char *path, int datasync, struct fuse_file_info *fi) {
        printf("fsync(path=%s, datasync=%d\n", path, datasync);
//	if(datasync) {
//		//sync data only
//		return RETURN_ERRNO(fdatasync(fi->fh));
//	} else {
//		//sync data + file metadata
//		return RETURN_ERRNO(fsync(fi->fh));
//	}
  return 0;
}

int DedupFS::Setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
//	printf("setxattr(path=%s, name=%s, value=%s, size=%d, flags=%d\n",
//		path, name, value, (int)size, flags);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(lsetxattr(fullPath, name, value, size, flags));
  return -ENOSYS;
}

int DedupFS::Getxattr(const char *path, const char *name, char *value, size_t size) {
//	printf("getxattr(path=%s, name=%s, size=%d\n", path, name, (int)size);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(getxattr(fullPath, name, value, size));
  return -ENOSYS;
}

int DedupFS::Listxattr(const char *path, char *list, size_t size) {
//	printf("listxattr(path=%s, size=%d)\n", path, (int)size);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(llistxattr(fullPath, list, size));
  return -ENOSYS;
}

int DedupFS::Removexattr(const char *path, const char *name) {
//	printf("removexattry(path=%s, name=%s)\n", path, name);
//	char fullPath[PATH_MAX];
//	AbsPath(fullPath, path);
//	return RETURN_ERRNO(lremovexattr(fullPath, name));
  return -ENOSYS;
}

int DedupFS::Opendir(const char *path, struct fuse_file_info *fileInfo) {
  printf("opendir(path=%s)", path);
  boost::intrusive_ptr<file_info> fi = db->getByPath(path);
  if (fi->st.st_ino) {
    if (!S_ISDIR(fi->st.st_mode)) {
      printf(": ENOTDIR %o\n", (int)fi->st.st_mode);
      return -ENOTDIR;
    }
    fileInfo->fh = (uint64_t)fi.detach();
    printf(": SUCCESS\n");
    return 0;
  } else {
    printf(": ENOENT\n");
    return -ENOENT;
  }
}

int DedupFS::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
  printf("readdir(path=%s, offset=%d)\n", path, (int)offset);
  boost::intrusive_ptr<file_info> fi = (file_info*)fileInfo->fh;
  std::vector<boost::intrusive_ptr<file_info> > files = db->readdir(fi);
  filler(buf, ".", &(fi->st), 0);
  filler(buf, "..", &(fi->st), 0);
  for (auto file = files.begin(); file != files.end(); ++file) {
    if (filler(buf, (*file)->name.leaf().c_str(), &((*file)->st), 0))
      break;
  }
  return 0;
}

int DedupFS::Releasedir(const char *path, struct fuse_file_info *fileInfo) {
  printf("releasedir(path=%s)\n", path);
  intrusive_ptr_release((file_info*)fileInfo->fh);
  return 0;
}

int DedupFS::Fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
	return 0;
}

void *DedupFS::Init(struct fuse_conn_info *conn) {
  return fuse_get_context()->private_data;
}

int DedupFS::Ftruncate(const char *path, off64_t newSize, struct fuse_file_info *fileInfo) {
  printf("ftruncate(path=%s, newSize=%ld)\n", path, newSize);
  return db->ftruncate((file_info*)fileInfo->fh, newSize);
}


