#pragma once

#include <opencv2/opencv.hpp>

#include "scene_analyzer/scene_analyzer.h"

class BattleSceneAnalyzer : public SceneAnalyzer::Interface {
 public:
  BattleSceneAnalyzer(const cv::Size& size);
  virtual ~BattleSceneAnalyzer();
  virtual void drawDebugInfo(cv::Mat* frame);
  virtual bool IsScene(const cv::Mat& frame);

 private:
  int mode_;
  cv::Rect clock_rec_;
  cv::Mat clock_;
};

