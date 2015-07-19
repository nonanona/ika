#include "scene_analyzer/scene_analyzer.h"

#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include "scene_analyzer/battle_scene_analyzer.h"
#include "scene_analyzer/result_scene_analyzer.h"
#include "scene_analyzer/blackout_scene_analyzer.h"

SceneAnalyzer::SceneAnalyzer(const cv::Size& size)
    : battle_guesser_(new BattleSceneAnalyzer()),
      result_guesser_(new ResultSceneAnalyzer()),
      blackout_guesser_(new BlackoutSceneAnalyzer(size)) {
}

SceneAnalyzer::~SceneAnalyzer() {
}

SceneAnalyzer::Scene SceneAnalyzer::Process(const cv::Mat& frame) {
  if (battle_guesser_->IsScene(frame)) {
    return BATTLE;
  } else if (result_guesser_->IsScene(frame)) {
    return RESULT;
  } else if (blackout_guesser_->IsScene(frame)) {
    return BLACKOUT;
  } else {
    return UNKNOWN;
  }
}

