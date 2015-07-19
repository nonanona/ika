#include <opencv2/opencv.hpp>

#include <glog/logging.h>

#include <string>
#include <vector>

#include "ocr/classifier.h"
#include "util.h"

Classifier::Classifier(const std::string& dir, double sigma) : sigma_(sigma) {
  Load(ScanDir(dir));
}

Classifier::~Classifier() {
  for (int i = 0; i < classifiers_.size(); ++i) {
    delete classifiers_[i];
  }
}

int Classifier::Predict(const std::string fname, double certainty) {
  cv::Mat data;
  ReadAndPrepare(fname, &data);
  return PredictInternal(data, certainty);
}

int Classifier::Predict(const cv::Mat& image, double certainty) {
  cv::Mat data;
  Flatten(image, &data);
  return PredictInternal(data, certainty);
}

int Classifier::PredictInternal(const cv::Mat& data, double certainty) {
  int counter[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = 0; i < classifiers_.size(); ++i) {
    counter[static_cast<int>(classifiers_[i]->predict(data))]++;
  }
  int max_label = -1;
  int max_freq = -1;
  for (int i = 0; i < 11; ++i) {
    if (max_freq < counter[i]) {
      max_label = i;
      max_freq = counter[i];
    }
  }
  double actual_certainity =
      static_cast<double>(max_freq) / static_cast<double>(classifiers_.size());

  if (actual_certainity < certainty) {
    LOG(ERROR) << "Unable to determine the label.";
    for (int i = 0; i < 11; ++i) {
      LOG(ERROR) << "  Label " << i << ": " << counter[i];
    }
    abort();
  }
  return max_label;
}

void Classifier::Load(const std::vector<std::string>& files) {
  if (files.empty())
    LOG(FATAL) << "Unable to load parameter files.";
  classifiers_.resize(files.size());
  for (int i = 0; i < files.size(); ++i) {
    LOG(INFO) << "Loading classifier params from: " << files[i];
    classifiers_[i] = new cv::SVM();
    classifiers_[i]->load(files[i].c_str());
  }
}

void Classifier::ReadAndPrepare(const std::string& fname, cv::Mat* out) {
  return Flatten(cv::imread(fname, CV_LOAD_IMAGE_GRAYSCALE), out);
}

void Classifier::Flatten(const cv::Mat& org_image, cv::Mat* out) {
  cv::Mat prepared_image;
  cv::GaussianBlur(org_image, prepared_image, cv::Size(0, 0), sigma_, sigma_);
  cv::Size size = prepared_image.size();
  int length = size.width * size.height;
  out->create(length, 1, CV_32FC1);
  int ii = 0;
  for (int j = 0; j < prepared_image.rows; ++j) {
    for (int k = 0; k < prepared_image.cols; ++k) {
      out->at<float>(ii++) = static_cast<float>(prepared_image.at<uchar>(j, k));
    }
  }
}
