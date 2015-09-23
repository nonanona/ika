#pragma once

#include <opencv2/opencv.hpp>

#include "scene_analyzer/scene_analyzer.h"

class ResultSceneAnalyzer : public SceneAnalyzer::Interface {
 public:
  ResultSceneAnalyzer(const cv::Size& size, bool is_camera);
  virtual ~ResultSceneAnalyzer();
  virtual void drawDebugInfo(cv::Mat* frame);
  virtual bool IsScene(const cv::Mat& frame);

 private:
  cv::Rect win_rec_;
  cv::Mat win_;
};

