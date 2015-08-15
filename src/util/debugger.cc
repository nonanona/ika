#include "util/debugger.h"

#define RESCALE 1

void ShowAndWaitKey(const cv::Mat& image) {
  static bool initialized = false;
  if (!initialized) {
    cv::namedWindow("debug", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    initialized = true;
  }
#ifdef RESCALE
  cv::Mat tmp;
  cv::Size size = image.size();
  cv::resize(image, tmp, cv::Size(size.width / 2, size.height / 2));
  cv::imshow("debug", tmp);
#else
  cv::imshow("debug", image);
#endif
  cv::waitKey(0);
}
