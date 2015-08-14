#include "util/ioutil.h"

#include <sys/stat.h>
#include <glog/logging.h>

bool IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return false;

  return st.st_mode & S_IFDIR;
}

bool IsFile(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) == -1)
    return false;
  return st.st_mode & S_IFREG;
}

bool FileExists(const std::string& path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

bool MakeDir(const std::string& path) {
  return mkdir(path.c_str(), 0755) == 0;
}

void MakeSureDirExists(const std::string& path) {
  if (!FileExists(path)) {
    MakeDir(path);
  }

  if (!IsDirectory(path)) {
    LOG(FATAL) << path << " is not a directory";
    abort();
  }
}
