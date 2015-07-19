#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

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
  std::vector<cv::SVM*> classifiers_;
};
