#pragma once

#include "scene_analyzer/scene_analyzer.h"

#include <opencv2/opencv.hpp>

class WhiteoutSceneAnalyzer : public SceneAnalyzer::Interface {
 public:
  WhiteoutSceneAnalyzer(const cv::Size& size);
  virtual ~WhiteoutSceneAnalyzer();
  virtual bool IsScene(const cv::Mat& frame);

 private:
  const cv::Rect rec_;
};

