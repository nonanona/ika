#include "ocr/paintpoint_classifier.h"

#include "util/debugger.h"
#include "glog/logging.h"
#include "util/ioutil.h"
#include "util/util.h"

PaintPointClassifier::PaintPointClassifier() {
  const char* fname = "res720/paintpoint_svm.xml";
  if (!FileExists(fname))
    LOG(FATAL) << fname << " missing.";

  classifier_ = cv::ml::StatModel::load<cv::ml::SVM>(fname);
}

PaintPointClassifier::~PaintPointClassifier() {
}

int PaintPointClassifier::Predict(const std::string& fname) const {
  return Predict(cv::imread(fname));
}

int PaintPointClassifier::Predict(const cv::Mat& image) const {
  cv::Mat tmp;
  ExtractWhite(image, &tmp);
  if (IsBlackImage(tmp))
    return 0;
  Prepare(image, &tmp);
  return classifier_->predict(tmp);
}

void PaintPointClassifier::Prepare(const cv::Mat& image, cv::Mat* out) {
  cv::Mat tmp;
  cv::resize(image, tmp, cv::Size(18, 20));
  
  cv::Mat tmp2;
  cv::cvtColor(tmp, tmp2, CV_RGB2GRAY);

  cv::Size size = tmp.size();
  *out = cv::Mat(1, size.width * size.height, CV_32FC1);
  int ii = 0;
  for (int i = 0; i < tmp2.rows; ++i) {
    for (int j = 0; j < tmp2.cols; ++j) {
      out->at<float>(0, ii++) = static_cast<float>(tmp2.at<uchar>(i, j)) / 255.0;
    }
  }
}
