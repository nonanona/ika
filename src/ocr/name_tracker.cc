#include "ocr/name_tracker.h"

#include <opencv2/opencv.hpp>

namespace {

double WhiteDiffInternal(const cv::Mat& img1, const cv::Mat& img2) {
int diff = 0;
  cv::Size img1_size = img1.size();
  cv::Size img2_size = img2.size();
  cv::Size smaller_size(std::min(img1_size.width, img2_size.width),
                        std::min(img1_size.height, img2_size.height));
  cv::Rect rect(cv::Point(0, 0), smaller_size);
  cv::Mat gray1;
  cv::Mat gray2;
  cv::cvtColor(img1(rect), gray1, CV_BGR2GRAY);
  cv::cvtColor(img2(rect), gray2, CV_BGR2GRAY);
  for (int i = 0; i < gray1.rows; ++i) {
    for (int j = 0; j < gray1.cols; ++j) {
      bool is_img_1_white = (gray1.at<uchar>(i, j) > 220);
      bool is_img_2_white = (gray2.at<uchar>(i, j) > 220);
      if (is_img_1_white != is_img_2_white) {
        diff++;
      }
    }
  }
  return (double)diff / (smaller_size.width * smaller_size.height);
}

double WhiteDiff(const cv::Mat& img1, const cv::Mat& img2) {
  cv::Size size1 = img1.size();
  cv::Size size2 = img2.size();

  double min_r = 1e+100;

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      double cand = WhiteDiffInternal(
          img1(cv::Rect(i, j, size1.width - i, size1.height -j)),
          img2(cv::Rect(0, 0, size2.width, size2.height)));

      if (cand < min_r)
        min_r = cand;

      cand = WhiteDiffInternal(
          img1(cv::Rect(0, 0, size1.width, size1.height)),
          img2(cv::Rect(i, j, size2.width - i, size2.height - j)));

      if (cand < min_r)
        min_r = cand;
    }
  }

  return min_r;
}

}  // namespace

NameTracker::NameTracker() : max_id_(0) {
}

NameTracker::~NameTracker() {
}

int NameTracker::GetNameId(const cv::Mat& name_image) {
  for(std::map<int, std::vector<cv::Mat> >::iterator it = name_map_.begin();
      it != name_map_.end(); ++it) {
    for (int i = 0; i < it->second.size(); ++i) {
      const cv::Mat& candidate = it->second[i];
      if (IsSameName(candidate, name_image)) {
        it->second.push_back(name_image);
        return it->first;
      }
    }
  }
  name_map_[max_id_] = std::vector<cv::Mat>();
  name_map_[max_id_].push_back(name_image);
  return max_id_++;
}

bool NameTracker::IsSameName(const cv::Mat& img1, const cv::Mat& img2) const {
  cv::Size size1 = img1.size();
  cv::Size size2 = img2.size();

  int abs_width_diff = abs(size1.width - size2.width);
  if (abs_width_diff > 5)
    return false;

  double white_diff = WhiteDiff(img1, img2);
  if (white_diff > 0.1)
    return false;

  return true;
}

