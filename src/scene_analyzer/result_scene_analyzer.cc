#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/result_scene_analyzer.h"

ResultSceneAnalyzer::ResultSceneAnalyzer()
    : win_rec_(990, 70, 140, 50),
      win_mask_(cv::imread("res/win.bmp", CV_LOAD_IMAGE_GRAYSCALE)),
      win_white_(cv::Size(140, 50), CV_8UC1, cv::Scalar(255, 255, 255)) {
}

ResultSceneAnalyzer::~ResultSceneAnalyzer() {
}

bool ResultSceneAnalyzer::IsScene(const cv::Mat& frame) {
  cv::Mat win(frame, win_rec_);
  cv::Mat win_gray;
  cvtColor(win, win_gray, CV_RGB2GRAY);
  cv::Mat win_gray_clipped = win_gray.clone();
  win_white_.copyTo(win_gray_clipped, win_mask_);

  cv::Size win_size = win.size();
  int win_sum = 0;
  int all_sum = 0;
  for (int i = 0; i < win_size.height; ++i) {
    const uchar* line1 = win_gray_clipped.ptr<uchar>(i);
    const uchar* line2 = win_gray.ptr<uchar>(i);
    for (int j = 0; j < win_size.width; ++j) {
      win_sum += 0xFF - line1[j];
      all_sum += 0xFF - line2[j];
    }
  }

  int avg_white = all_sum / (win_size.width * win_size.height);

  return win_sum < 10000 && avg_white > 10;
}
