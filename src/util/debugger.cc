#include "util/debugger.h"

void ShowAndWaitKey(const cv::Mat& image) {
  static bool initialized = false;
  if (!initialized) {
    cv::namedWindow("debug", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    initialized = true;
  }
  cv::imshow("debug", image);
  cv::waitKey(0);
}
