#include "ocr/killdeath_classifier.h"

#include "glog/logging.h"
#include "util/ioutil.h"
#include "util/util.h"

KillDeathClassifier::KillDeathClassifier() {
  const char* fname = "res720/killdeath_svm.xml";
  if (!FileExists(fname))
    LOG(FATAL) << fname << " missing.";

  classifier_ = cv::ml::StatModel::load<cv::ml::SVM>(fname);
}

KillDeathClassifier::~KillDeathClassifier() {
}

int KillDeathClassifier::Predict(const std::string& fname) const {
  return Predict(cv::imread(fname));
}

int KillDeathClassifier::Predict(const cv::Mat& image) const {
  cv::Mat tmp;
  ExtractWhite(image, &tmp);
  if (IsBlackImage(tmp))
    return 0;
  Prepare(image, &tmp);
  return classifier_->predict(tmp);
}

void KillDeathClassifier::Prepare(const cv::Mat& image, cv::Mat* out) {
  cv::Mat tmp;
  cv::resize(image, tmp, cv::Size(15, 17));

  cv::Mat tmp2;
  cv::cvtColor(tmp, tmp2, CV_RGB2GRAY);

  cv::Size size = tmp.size();
  *out = cv::Mat(1, size.width * size.height, CV_32FC1);
  int ii = 0;
  for (int i = 0; i < tmp.rows; ++i) {
    for (int j = 0; j < tmp.cols; ++j) {
      out->at<float>(0, ii++) = static_cast<float>(tmp2.at<uchar>(i, j)) / 255.0;
    }
  }
}
