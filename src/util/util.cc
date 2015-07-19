#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <glog/logging.h>

#include <string>
#include <vector>

void PrintRect(const cv::Rect& rect) {
  printf("(%d, %d) - %dx%d\n", rect.x, rect.y, rect.width, rect.height);
}

void PrintSize(const cv::Size& size) {
  printf("%dx%d\n", size.width, size.height);
}

void PrintPoint(const cv::Point& pos) {
  printf("(%d, %d)\n", pos.x, pos.y);
}

std::vector<std::string> ScanDir(const std::string& dir) {
  struct dirent **namelist;
  int n = scandir(dir.c_str(), &namelist, NULL, alphasort);
  std::vector<std::string> out;

  for (int i = 0; i < n; ++i) {
    if (namelist[i]->d_name[0] == '.')
      continue;
    out.push_back(dir + "/" + namelist[i]->d_name);
    free(namelist[i]);
  }
  return out;
}

std::string GetCmdOption(char** begin, char** end, const std::string& opt) {
  char** it = std::find(begin, end, opt);
  if (it != end && ++it != end)
    return std::string(*it);
  return std::string();
}

bool HasCmdOption(char** begin, char** end, const std::string& opt) {
  return std::find(begin, end, opt) != end;
}
