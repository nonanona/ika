#pragma once
#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

class ClassifierInterface {
 public:
  ClassifierInterface() {}
  virtual ~ClassifierInterface() {}

  virtual int Predict(const std::string& fname) const = 0;
  virtual int Predict(const cv::Mat& image) const = 0;
};
