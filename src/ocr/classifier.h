#pragma once
#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#if CV_MAJOR_VERSION == 3
typedef cv::Ptr<cv::ml::SVM> MySVM;
#else
typedef cv::SVM* MySVM;
#endif

class Classifier {
 public:
  Classifier(const std::string& dir, double sigma);
  ~Classifier();

  int Predict(const std::string fname, double certainty);
  int Predict(const cv::Mat& image, double certainty);

 private:
  int PredictInternal(const cv::Mat& mat, double certainty);
  void Load(const std::vector<std::string>& files);
  void ReadAndPrepare(const std::string& fname, cv::Mat* out);
  void Flatten(const cv::Mat& image, cv::Mat* out);

  const double sigma_;
  std::vector<MySVM> classifiers_;
};
