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

void ExtractWhite(const cv::Mat& in, cv::Mat* out) {
  cv::Mat tmp;
  cv::cvtColor(in, tmp, CV_RGB2GRAY);
  cv::threshold(tmp, *out, 0xE0, 0xFF, CV_THRESH_BINARY);
}

bool IsBlackImage(const cv::Mat& in) {
  for (int i = 0; i < in.rows;++i) {
    for (int j = 0; j < in.cols; ++j) {
      if (in.at<uchar>(i, j) != 0)
        return false;
    }
  }
  return true;
}
bool IsWhiteImage(const cv::Mat& in) {
  for (int i = 0; i < in.rows;++i) {
    for (int j = 0; j < in.cols; ++j) {
      if (in.at<uchar>(i, j) != 0xFF)
        return false;
    }
  }
  return true;
}

double ImageDiff(const cv::Mat& l, const cv::Mat& r) {
  int diff_cnt = 0;
  for (int i = 0; i < l.rows; ++i) {
    for (int j = 0; j < l.cols; ++j) {
      if (l.at<uchar>(i, j) != r.at<uchar>(i, j)) {
        diff_cnt ++;
      }
    }
  }
  return (double)diff_cnt / (double)(l.rows * l.cols);
}
