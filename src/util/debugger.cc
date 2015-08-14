#include "util/debugger.h"

void ShowAndWaitKey(const cv::Mat& image) {
  static bool initialized = false;
  if (!initialized) {
    cv::namedWindow("debug", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    initialized = true;
  }
  cv::Size org = image.size();
  cv::Mat image2;
  cv::resize(image, image2, cv::Size(org.width / 2, org.height / 2));
  cv::imshow("debug", image2);
  cv::waitKey(0);
}
