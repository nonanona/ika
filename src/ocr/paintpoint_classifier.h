#pragma once

#include "ocr/classifier_interface.h"

class PaintPointClassifier : public ClassifierInterface {
 public:
  PaintPointClassifier();
  ~PaintPointClassifier();

  virtual int Predict(const std::string& fname) const;
  virtual int Predict(const cv::Mat& image) const;
  static void Prepare(const cv::Mat& image, cv::Mat* out);

 private:
  cv::Ptr<cv::ml::SVM> classifier_;
};
