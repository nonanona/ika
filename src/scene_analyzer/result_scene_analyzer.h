#pragma once

#include <opencv2/opencv.hpp>

#include "scene_analyzer/scene_analyzer.h"

class ResultSceneAnalyzer : public SceneAnalyzer::Interface {
 public:
  ResultSceneAnalyzer();
  virtual ~ResultSceneAnalyzer();
  virtual bool IsScene(const cv::Mat& frame);

 private:
  const cv::Rect win_rec_;
  const cv::Mat win_white_;
  const cv::Mat win_mask_;
};

