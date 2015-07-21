#pragma once

#include "ocr/classifier.h"
#include "ocr/image_clipper.h"

class ResultPageReader {
 public:
  ResultPageReader(bool is_nawabri);
  ~ResultPageReader();

  void LoadImage(const cv::Mat& image);

  int ReadKillCount(int index) const;
  int ReadDeathCount(int index) const;
  int ReadPaintPoint(int index) const;

  void ShowDebugImage(bool with_rect) const;

  bool is_nawabari() const { return is_nawabari_; }

 private:
  void PredictImages(const cv::Mat* images, int* out, int length,
                     Classifier* classifier) const;

  cv::Mat image_;
  const bool is_nawabari_;
  ImageClipper* image_clipper_;
  Classifier* killdeath_classifier_;
  Classifier* paintpoint_classifier_;
};
