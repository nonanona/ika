#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/blackout_scene_analyzer.h"
#include "util/util.h"

#define CLIP_WIDTH 200
#define CLIP_HEIGHT 200

BlackoutSceneAnalyzer::BlackoutSceneAnalyzer(const cv::Size& size)
    : rec_((size.width - CLIP_WIDTH) / 2, (size.height - CLIP_WIDTH) / 2,
           CLIP_WIDTH, CLIP_HEIGHT) {
}

BlackoutSceneAnalyzer::~BlackoutSceneAnalyzer() {
}

void BlackoutSceneAnalyzer::drawDebugInfo(cv::Mat* frame) {
}

bool BlackoutSceneAnalyzer::IsScene(const cv::Mat& frame) {
  cv::Mat win(frame, rec_);

  cv::Mat tmp;
  cv::Mat tmp2;
  cv::cvtColor(win, tmp, CV_RGB2GRAY);
  cv::threshold(tmp, tmp2, 0x10, 0xFF, CV_THRESH_BINARY);
  return IsBlackImage(tmp2);
}
