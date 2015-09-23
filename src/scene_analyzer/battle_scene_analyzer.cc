#include <opencv2/opencv.hpp>

#include "util/debugger.h"
#include "util/util.h"
#include "glog/logging.h"
#include "scene_analyzer/battle_scene_analyzer.h"

BattleSceneAnalyzer::BattleSceneAnalyzer(const cv::Size& size, bool is_camera) {
  if (size.width == 1920 && size.height == 1080) {
    clock_rec_ = cv::Rect(89, 49, 44, 51);
    clock_ = cv::imread("res/clock.png", CV_LOAD_IMAGE_GRAYSCALE);
  } else if (size.width == 1280 && size.height == 720) {
    clock_rec_ = cv::Rect(53, 30, 40, 40);
    clock_ = cv::imread(
        is_camera ? "res720camera/clock.png" : "res720/clock.png",
        CV_LOAD_IMAGE_GRAYSCALE);
  } else {
    LOG(ERROR) << "Unsupported image size: "
        << size.width << "x" << size.height;
    abort();
  }
}

BattleSceneAnalyzer::~BattleSceneAnalyzer() {
}

void BattleSceneAnalyzer::drawDebugInfo(cv::Mat* frame) {
  cv::rectangle(*frame, clock_rec_.tl(), clock_rec_.br(),
                cv::Scalar(0x00, 0xFF, 0xFF), 2, 8);
}

bool BattleSceneAnalyzer::IsScene(const cv::Mat& frame_img) {
  cv::Mat clock(frame_img, clock_rec_);
  cv::Mat clock_gray;
  ExtractWhite(clock, &clock_gray);
  if (IsBlackImage(clock_gray))
    return false;

  cv::Size size = clock_gray.size();
  double r = ImageDiff(clock_, clock_gray);
  return r < 0.05;
}
