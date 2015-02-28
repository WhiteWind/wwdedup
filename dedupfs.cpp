#include "dedupfs.h"

DedupFS* DedupFS::_instance = NULL;

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

DedupFS* DedupFS::Instance() {
	if(_instance == NULL) {
                _instance = new DedupFS();
	}
	return _instance;
}

DedupFS::DedupFS() {

}

DedupFS::~DedupFS() {

}

void DedupFS::AbsPath(char dest[PATH_MAX], const char *path) {
	strcpy(dest, _root);
	strncat(dest, path, PATH_MAX);
	//printf("translated path: %s to %s\n", path, dest);
}

void DedupFS::setRootDir(const char *path) {
	printf("setting FS root to: %s\n", path);
	_root = path;
}

int DedupFS::Getattr(const char *path, struct stat *statbuf) {
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	printf("getattr(%s)\n", fullPath);
	return RETURN_ERRNO(lstat(fullPath, statbuf));
}

int DedupFS::Readlink(const char *path, char *link, size_t size) {
	printf("readlink(path=%s, link=%s, size=%d)\n", path, link, (int)size);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(readlink(fullPath, link, size));
}

int DedupFS::Mknod(const char *path, mode_t mode, dev_t dev) {
	printf("mknod(path=%s, mode=%d)\n", path, mode);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	
	//handles creating FIFOs, regular files, etc...
	return RETURN_ERRNO(mknod(fullPath, mode, dev));
}

int DedupFS::Mkdir(const char *path, mode_t mode) {
	printf("**mkdir(path=%s, mode=%d)\n", path, (int)mode);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(mkdir(fullPath, mode));
}

int DedupFS::Unlink(const char *path) {
	printf("unlink(path=%s\n)", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(unlink(fullPath));
}

int DedupFS::Rmdir(const char *path) {
	printf("rmkdir(path=%s\n)", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(rmdir(fullPath));
}

int DedupFS::Symlink(const char *path, const char *link) {
	printf("symlink(path=%s, link=%s)\n", path, link);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(symlink(fullPath, link));
}

int DedupFS::Rename(const char *path, const char *newpath) {
	printf("rename(path=%s, newPath=%s)\n", path, newpath);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(rename(fullPath, newpath));
}

int DedupFS::Link(const char *path, const char *newpath) {
	printf("link(path=%s, newPath=%s)\n", path, newpath);
	char fullPath[PATH_MAX];
	char fullNewPath[PATH_MAX];
	AbsPath(fullPath, path);
	AbsPath(fullNewPath, newpath);
	return RETURN_ERRNO(link(fullPath, fullNewPath));
}

int DedupFS::Chmod(const char *path, mode_t mode) {
	printf("chmod(path=%s, mode=%d)\n", path, mode);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(chmod(fullPath, mode));
}

int DedupFS::Chown(const char *path, uid_t uid, gid_t gid) {
	printf("chown(path=%s, uid=%d, gid=%d)\n", path, (int)uid, (int)gid);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(chown(fullPath, uid, gid));
}

int DedupFS::Truncate(const char *path, off_t newSize) {
	printf("truncate(path=%s, newSize=%d\n", path, (int)newSize);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(truncate(fullPath, newSize));
}

int DedupFS::Utime(const char *path, struct utimbuf *ubuf) {
	printf("utime(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(utime(fullPath, ubuf));
}

int DedupFS::Open(const char *path, struct fuse_file_info *fileInfo) {
	printf("open(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	fileInfo->fh = open(fullPath, fileInfo->flags);
	return 0;
}

int DedupFS::Create(const char *path, mode_t mode, struct fuse_file_info *fileInfo)
{

}

int DedupFS::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	printf("read(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
	return RETURN_ERRNO(pread(fileInfo->fh, buf, size, offset));
}

int DedupFS::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	printf("write(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
	return RETURN_ERRNO(pwrite(fileInfo->fh, buf, size, offset));
}

int DedupFS::Statfs(const char *path, struct statvfs *statInfo) {
	printf("statfs(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(statvfs(fullPath, statInfo));
}

int DedupFS::Flush(const char *path, struct fuse_file_info *fileInfo) {
	printf("flush(path=%s)\n", path);
	//noop because we don't maintain our own buffers
	return 0;
}

int DedupFS::Release(const char *path, struct fuse_file_info *fileInfo) {
	printf("release(path=%s)\n", path);
	return 0;
}

int DedupFS::Fsync(const char *path, int datasync, struct fuse_file_info *fi) {
	printf("fsync(path=%s, datasync=%d\n", path, datasync);
	if(datasync) {
		//sync data only
		return RETURN_ERRNO(fdatasync(fi->fh));
	} else {
		//sync data + file metadata
		return RETURN_ERRNO(fsync(fi->fh));
	}
}

int DedupFS::Setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
	printf("setxattr(path=%s, name=%s, value=%s, size=%d, flags=%d\n",
		path, name, value, (int)size, flags);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(lsetxattr(fullPath, name, value, size, flags));
}

int DedupFS::Getxattr(const char *path, const char *name, char *value, size_t size) {
	printf("getxattr(path=%s, name=%s, size=%d\n", path, name, (int)size);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(getxattr(fullPath, name, value, size));
}

int DedupFS::Listxattr(const char *path, char *list, size_t size) {
	printf("listxattr(path=%s, size=%d)\n", path, (int)size);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(llistxattr(fullPath, list, size));
}

int DedupFS::Removexattr(const char *path, const char *name) {
	printf("removexattry(path=%s, name=%s)\n", path, name);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(lremovexattr(fullPath, name));
}

int DedupFS::Opendir(const char *path, struct fuse_file_info *fileInfo) {
	printf("opendir(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	DIR *dir = opendir(fullPath);
	fileInfo->fh = (uint64_t)dir;
	return NULL == dir ? -errno : 0;
}

int DedupFS::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
	printf("readdir(path=%s, offset=%d)\n", path, (int)offset);
	DIR *dir = (DIR*)fileInfo->fh;
	struct dirent *de = readdir(dir);
	if(NULL == de) {
		return -errno;
	} else {
		do {
			if(filler(buf, de->d_name, NULL, 0) != 0) {
				return -ENOMEM;
			}
		} while(NULL != (de = readdir(dir)));
	}
	return 0;
}

int DedupFS::Releasedir(const char *path, struct fuse_file_info *fileInfo) {
	printf("releasedir(path=%s)\n", path);
	closedir((DIR*)fileInfo->fh);
	return 0;
}

int DedupFS::Fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
	return 0;
}

void *DedupFS::Init(struct fuse_conn_info *conn) {
	return 0;
}

int DedupFS::Truncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
	printf("truncate(path=%s, offset=%d)\n", path, (int)offset);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(ftruncate(fileInfo->fh, offset));
}


