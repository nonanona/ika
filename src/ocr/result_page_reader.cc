#include "ocr/result_page_reader.h"

ResultPageReader::ResultPageReader(bool is_nawabari)
    : is_nawabari_(is_nawabari),
      image_clipper_(NULL),
      killdeath_classifier_(
          new Classifier("res/kill_death_param", 1.1)),
      paintpoint_classifier_(
          is_nawabari ? new Classifier("res/point_param", 2.0) : NULL) {
}

ResultPageReader::~ResultPageReader() {
  delete paintpoint_classifier_;
  delete killdeath_classifier_;
  delete image_clipper_;
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
    out[i] = classifier->Predict(buf, 0.8);
  }
}

int ResultPageReader::ReadKillCount(int index) const {
  const ImageClipper::ClippedImage& image = image_clipper_->getImage(index);
  cv::Mat buf;
  int result[2] = {};
  PredictImages(image.kill, result, 2, killdeath_classifier_);
  return (result[0] % 10) * 10 + (result[1] % 10);
}

int ResultPageReader::ReadDeathCount(int index) const {
  const ImageClipper::ClippedImage& image = image_clipper_->getImage(index);
  cv::Mat buf;
  int result[2] = {};
  PredictImages(image.death, result, 2, killdeath_classifier_);
  return (result[0] % 10) * 10 + (result[1] % 10);
}

int ResultPageReader::ReadPaintPoint(int index) const {
  const ImageClipper::ClippedImage& image = image_clipper_->getImage(index);
  cv::Mat buf;
  int result[4] = {};
  PredictImages(image.point, result, 4, paintpoint_classifier_);
  return (result[0] % 10) * 1000 + (result[1] % 10) * 100 +
      (result[2] % 10) * 10 + (result[3] % 10);
}

void ResultPageReader::ShowDebugImage(bool with_rect) const {
  image_clipper_->ShowDebugImage(with_rect);
}
