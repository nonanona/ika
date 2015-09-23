#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "util/util.h"
#include "util/debugger.h"
#include "scene_analyzer/result_scene_analyzer.h"
#include "glog/logging.h"
#include "ocr/image_clipper.h"

ResultSceneAnalyzer::ResultSceneAnalyzer(const cv::Size& size, bool is_camera) {
  if (size.width == 1920 && size.height == 1080) {
    win_rec_ = cv::Rect(990, 70, 140, 50);
    win_ = cv::imread("res/win.png", CV_LOAD_IMAGE_GRAYSCALE);
  } else if (size.width == 1280 && size.height == 720) {
    win_rec_ = cv::Rect(660, 47, 93, 33);
    win_ = cv::imread("res720/win.png", CV_LOAD_IMAGE_GRAYSCALE);
  } else {
    LOG(ERROR) << "Unsupported image size: "
        << size.width << "x" << size.height;
    abort();
  }
}

ResultSceneAnalyzer::~ResultSceneAnalyzer() {
}

void ResultSceneAnalyzer::drawDebugInfo(cv::Mat* frame) {
  cv::rectangle(*frame, win_rec_.tl(), win_rec_.br(),
                cv::Scalar(0x00, 0xFF, 0xFF), 2, 8);
}

bool ResultSceneAnalyzer::IsScene(const cv::Mat& frame) {
  cv::Mat win(frame, win_rec_);
  cv::Mat win_gray;
  ExtractWhite(win, &win_gray);
  if (IsBlackImage(win_gray)) {
    return false;
  }
  double r = ImageDiff(win_, win_gray);
  return r < 0.05;
}
