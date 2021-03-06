#include "ocr/result_page_reader.h"

#include "util/util.h"

ResultPageReader::ResultPageReader()
    : image_clipper_(NULL) {
}

ResultPageReader::~ResultPageReader() {
  delete image_clipper_;
}

void ResultPageReader::LoadImage(const cv::Mat& image, bool is_camera) {
  if (image_clipper_)
    delete image_clipper_;
  image_clipper_ = new ImageClipper(image, is_camera);
}

int ResultPageReader::ReadKillCount(int index) const {
  const ImageClipper::ClippedImage& image = image_clipper_->getImage(index);
  cv::Mat buf;
  int result[2] = {};
  result[0] = killdeath_classifier_.Predict(image.kill[0]);
  result[1] = killdeath_classifier_.Predict(image.kill[1]);
  return result[0] * 10 + result[1];
}

int ResultPageReader::ReadDeathCount(int index) const {
  const ImageClipper::ClippedImage& image = image_clipper_->getImage(index);
  cv::Mat buf;
  int result[2] = {};
  result[0] = killdeath_classifier_.Predict(image.death[0]);
  result[1] = killdeath_classifier_.Predict(image.death[1]);
  return result[0] * 10 + result[1];
}

int ResultPageReader::ReadPaintPoint(int index) const {
  const ImageClipper::ClippedImage& image = image_clipper_->getImage(index);
  cv::Mat buf;
  int result[4] = {};
  for (int i = 0; i < 4; ++i) {
    result[i] = paintpoint_classifier_.Predict(image.point[i]);
  }
  return result[0] * 1000 + result[1] * 100 + result[2] * 10 + result[3];
}

ImageClipper::PlayerStatus ResultPageReader::GetPlayerStatus(int i) const {
  return image_clipper_->GetPlayerStatus(i);
}

const cv::Mat& ResultPageReader::GetNameImage(int index) const {
  return image_clipper_->getImage(index).name;
}
