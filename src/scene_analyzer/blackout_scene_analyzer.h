#pragma once

#include "scene_analyzer/scene_analyzer.h"

#include <opencv2/opencv.hpp>

class BlackoutSceneAnalyzer : public SceneAnalyzer::Interface {
 public:
  BlackoutSceneAnalyzer(const cv::Size& size);
  virtual ~BlackoutSceneAnalyzer();
  virtual bool IsScene(const cv::Mat& frame);

 private:
  const cv::Rect rec_;
};

