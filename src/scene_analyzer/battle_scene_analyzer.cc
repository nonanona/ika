#include <opencv2/opencv.hpp>

#include "glog/logging.h"
#include "scene_analyzer/battle_scene_analyzer.h"

BattleSceneAnalyzer::BattleSceneAnalyzer()
    : clock_rec_(80, 45, 60, 60),
      clock_white_(cv::imread("res/clock_white.bmp", CV_LOAD_IMAGE_GRAYSCALE)),
      clock_black_(cv::imread("res/clock_black.bmp", CV_LOAD_IMAGE_GRAYSCALE)) {
}

BattleSceneAnalyzer::~BattleSceneAnalyzer() {
}

bool BattleSceneAnalyzer::IsScene(const cv::Mat& frame_img) {
  cv::Mat clock(frame_img, clock_rec_);
  cv::Mat clock_gray;
  cvtColor(clock, clock_gray, CV_RGB2GRAY);

  cv::Size clock_size = clock_gray.size();
  int white_diff = 0;
  int black_diff = 0;
  int total_white_cnt = 0;
  int total_black_cnt = 0;
  for (int i = 0; i < clock_size.height; ++i) {
    const uchar* line_white = clock_white_.ptr<uchar>(i);
    const uchar* line_black = clock_black_.ptr<uchar>(i);
    const uchar* line_org = clock_gray.ptr<uchar>(i);
    for (int j = 0; j < clock_size.width; ++j) {
      if (line_white[j] == 0xFF) {
        white_diff += (0xFF - line_org[j]) * (0xFF - line_org[j]);
        total_white_cnt++;
      }
      if (line_black[j] == 0x00) {
        black_diff += line_org[j] * line_org[j];
        total_black_cnt++;
      }
    }
  }
  double white_diff_ratio =
      static_cast<double>(white_diff) / (65025.0 * total_white_cnt);
  double black_diff_ratio =
      static_cast<double>(black_diff) / (65025.0 * total_black_cnt);
  return (white_diff_ratio < 0.05 && black_diff_ratio < 0.05);
}
