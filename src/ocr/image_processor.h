#pragma once

#include <opencv2/opencv.hpp>

#include <string>

class Classifier;

class ImageProcessor {
 public:
  ImageProcessor(const std::string& fname, bool is_nawabari);
  ImageProcessor(const cv::Mat& image, bool is_nawabari);
  virtual ~ImageProcessor();

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

 private:
  void calculateRect();
  void calculateRectInternal(int i, bool is_player);
  void clippingImage();
  void clippingImageInternal(int i);

  const bool is_nawabari_;
  cv::Mat image_;
  cv::Mat gray_image_;
  ClippingRect rects_[8];
  ClippedImage images_[8];
};