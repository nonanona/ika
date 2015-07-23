#include "scene_analyzer/scene_analyzer.h"

#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/battle_scene_analyzer.h"
#include "scene_analyzer/result_scene_analyzer.h"
#include "scene_analyzer/blackout_scene_analyzer.h"

SceneAnalyzer::SceneAnalyzer(const cv::Size& size)
    : battle_analyzer_(new BattleSceneAnalyzer()),
      result_analyzer_(new ResultSceneAnalyzer()),
      blackout_analyzer_(new BlackoutSceneAnalyzer(size)) {
}

SceneAnalyzer::~SceneAnalyzer() {
}

bool SceneAnalyzer::IsBattleScene(const cv::Mat& frame) {
  return battle_analyzer_->IsScene(frame);
}

bool SceneAnalyzer::IsResultScene(const cv::Mat& frame) {
  return result_analyzer_->IsScene(frame);
}

bool SceneAnalyzer::IsBlackoutScene(const cv::Mat& frame) {
  return blackout_analyzer_->IsScene(frame);
}
