#pragma once

#include <opencv2/opencv.hpp>

#include <string>

class Classifier;

class ImageClipper {
 public:
  ImageClipper(const std::string& fname, bool is_nawabari);
  ImageClipper(const cv::Mat& image, bool is_nawabari);
  virtual ~ImageClipper();

  enum PlayerStatus {
    NORMAL = 0,
    VACANCY = -1,
    YOU = 1,
  };

  struct ClippingRect {
    cv::Rect result;
    cv::Rect name;
    cv::Rect point[4];
    cv::Rect kill[2];
    cv::Rect death[2];
  };

  struct ClippedImage {
    cv::Mat result;
    cv::Mat name;
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

  void ShowDebugImage(bool with_rect) const;

  PlayerStatus GetPlayerStatus(int i ) const {
    return status_[i];
  }

 private:
  void calculateRect();
  void calculateRectInternal(int i, bool is_player);
  void clippingImage();
  void clippingImageInternal(int i);

  const bool is_nawabari_;
  PlayerStatus status_[8];
  cv::Mat image_;
  cv::Mat gray_image_;
  ClippingRect rects_[8];
  ClippedImage images_[8];
};
