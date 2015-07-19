#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/blackout_scene_analyzer.h"

#define CLIP_WIDTH 100
#define CLIP_HEIGHT 100

BlackoutSceneAnalyzer::BlackoutSceneAnalyzer(const cv::Size& size)
    : rec_((size.width - CLIP_WIDTH) / 2, (size.height - CLIP_WIDTH) / 2,
           CLIP_WIDTH, CLIP_HEIGHT) {
}

BlackoutSceneAnalyzer::~BlackoutSceneAnalyzer() {
}

bool BlackoutSceneAnalyzer::IsScene(const cv::Mat& frame) {
  cv::Mat win(frame, rec_);

  cv::Size win_size = win.size();
  int win_sum = 0;
  for (int i = 0; i < win_size.height; ++i) {
    const uchar* line1 = win.ptr<uchar>(i);
    for (int j = 0; j < win_size.width; ++j) {
      win_sum += line1[j];
    }
  }

  double avg = static_cast<double>(win_sum)
      / static_cast<double>(win_size.width * win_size.height);

  return avg < 3.0;
}
