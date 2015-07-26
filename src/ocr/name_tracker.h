#pragma once

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <map>

class NameTracker {
 public:
  NameTracker();
  ~NameTracker();

  int GetNameId(const cv::Mat& name_image);

  bool IsSameName(const cv::Mat& img1, const cv::Mat& img2) const;

 private:
  std::map<int, std::vector<cv::Mat> > name_map_;
  int max_id_;
};
