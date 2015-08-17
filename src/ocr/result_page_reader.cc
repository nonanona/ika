#include "ocr/result_page_reader.h"

#include "util/util.h"

ResultPageReader::ResultPageReader()
    : is_nawabari_(false),
      image_clipper_(NULL) {
}

ResultPageReader::~ResultPageReader() {
  delete image_clipper_;
}

void ResultPageReader::SetIsNawabari(bool is_nawabari) {
  is_nawabari_ = is_nawabari;
}

void ResultPageReader::LoadImage(const cv::Mat& image) {
  if (image_clipper_)
    delete image_clipper_;
  image_clipper_ = new ImageClipper(image, is_nawabari_);
}

void ResultPageReader::PredictImages(
    const cv::Mat* images, int* out, int length, Classifier* classifier) const {
  cv::Mat buf;
  for (int i = 0; i < length; ++i) {
    cv::Canny(images[i], buf, 50, 200);
    out[i] = classifier->Predict(buf, 0.7);
  }
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

void ResultPageReader::ShowDebugImage(bool with_rect) const {
  image_clipper_->ShowDebugImage(with_rect);
}

const cv::Mat& ResultPageReader::GetNameImage(int index) const {
  return image_clipper_->getImage(index).name;
}
