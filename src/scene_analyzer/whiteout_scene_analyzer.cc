#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/whiteout_scene_analyzer.h"
#include "glog/logging.h"

#define CLIP_WIDTH 100
#define CLIP_HEIGHT 100

WhiteoutSceneAnalyzer::WhiteoutSceneAnalyzer(const cv::Size& size)
    : rec_((size.width - CLIP_WIDTH) / 2, (size.height - CLIP_WIDTH) / 2,
           CLIP_WIDTH, CLIP_HEIGHT) {
}

WhiteoutSceneAnalyzer::~WhiteoutSceneAnalyzer() {
}

void WhiteoutSceneAnalyzer::drawDebugInfo(cv::Mat* frame) {
}

bool WhiteoutSceneAnalyzer::IsScene(const cv::Mat& frame) {
  cv::Mat win(frame, rec_);

  cv::Size win_size = win.size();
  int win_sum = 0;
  for (int i = 0; i < win_size.height; ++i) {
    const uchar* line1 = win.ptr<uchar>(i);
    for (int j = 0; j < win_size.width; ++j) {
      win_sum += 0xFF - line1[j];
    }
  }

  double avg = static_cast<double>(win_sum)
      / static_cast<double>(win_size.width * win_size.height);

  return avg < 5.0;
}
