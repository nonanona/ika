#pragma once

#include "ocr/killdeath_classifier.h"
#include "ocr/paintpoint_classifier.h"
#include "ocr/image_clipper.h"

class ResultPageReader {
 public:
  ResultPageReader();
  ~ResultPageReader();

  void LoadImage(const cv::Mat& image);

  int ReadKillCount(int index) const;
  int ReadDeathCount(int index) const;
  int ReadPaintPoint(int index) const;
  const cv::Mat& GetNameImage(int index) const;
  ImageClipper::PlayerStatus GetPlayerStatus(int i) const;

  bool IsNawabari() { return image_clipper_->IsNawabari(); }

 private:
  cv::Mat image_;
  ImageClipper* image_clipper_;
  KillDeathClassifier killdeath_classifier_;
  PaintPointClassifier paintpoint_classifier_;
};
