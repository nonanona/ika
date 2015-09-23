#pragma once

#include <opencv2/opencv.hpp>

#include <string>

class Classifier;

class ImageClipper {
 public:
  ImageClipper(const std::string& fname, bool is_camera);
  ImageClipper(const cv::Mat& image, bool is_camera);
  virtual ~ImageClipper();

  enum PlayerStatus {
    NORMAL = 0,
    VACANCY = -1,
    YOU = 1,
  };

  struct ClippingRect {
    cv::Rect frame;
    cv::Rect name;
    cv::Rect weapon;
    cv::Rect point[4];
    cv::Rect kill[2];
    cv::Rect death[2];
  };

  struct ClippedImage {
    cv::Mat result;
    cv::Mat name;
    cv::Mat weapon;
    cv::Mat point[4];
    cv::Mat kill[2];
    cv::Mat death[2];
  };

  const ClippingRect& getRect(int index) const {
    return rects_[index];
  }

  const ClippedImage& getImage(int index) const {
    return images_[index];
  }

  bool IsNawabari() const {
    return is_nawabari_;
  }

  void ShowDebugImage(const cv::Mat& image) const;
  cv::Mat DrawDebugInfo(const cv::Mat& image) const;

  PlayerStatus GetPlayerStatus(int i ) const {
    return status_[i];
  }

 private:
  void calcRect(const cv::Mat& image, bool is_camera);
  void calcRect720(const cv::Mat& image, bool is_camera);
  void calcRect1080(const cv::Mat& image);
  void clipImage(const cv::Mat& image);

  bool is_nawabari_;
  PlayerStatus status_[8];
  ClippingRect rects_[8];
  ClippedImage images_[8];
};
