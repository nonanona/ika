#pragma once

#include <opencv2/opencv.hpp>

#include "scene_analyzer/scene_analyzer.h"

class BattleSceneAnalyzer : public SceneAnalyzer::Interface {
 public:
  BattleSceneAnalyzer();
  virtual ~BattleSceneAnalyzer();
  virtual bool IsScene(const cv::Mat& frame);

 private:
  const cv::Rect clock_rec_;
  const cv::Mat clock_black_;
  const cv::Mat clock_white_;
  const cv::Mat clock_mask_;
};

