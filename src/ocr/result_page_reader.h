#pragma once

#include "ocr/classifier.h"
#include "ocr/image_clipper.h"

class ResultPageReader {
 public:
  ResultPageReader(bool is_nawabri);
  ~ResultPageReader();

  void LoadImage(const cv::Mat& image);

  int ReadKillCount(int index);
  int ReadDeathCount(int index);
  int ReadPaintPoint(int index);

  void ShowDebugImage(bool with_rect) const;

 private:
  void PredictImages(
      const cv::Mat* images, int* out, int length, Classifier* classifier);

  cv::Mat image_;
  const bool is_nawabari_;
  ImageClipper* image_clipper_;
  Classifier* killdeath_classifier_;
  Classifier* paintpoint_classifier_;
};
