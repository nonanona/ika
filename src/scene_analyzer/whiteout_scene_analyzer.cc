#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/whiteout_scene_analyzer.h"
#include "util/util.h"

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

  cv::Mat tmp;
  ExtractWhite(win, &tmp);
  return IsWhiteImage(tmp);
}
