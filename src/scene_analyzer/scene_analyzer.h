#pragma once

#include <opencv2/opencv.hpp>

class SceneAnalyzer {
 public:
  SceneAnalyzer(const cv::Size& size);
  virtual ~SceneAnalyzer();

  class Interface {
   public:
    Interface() {}
    virtual ~Interface() {}
    virtual void drawDebugInfo(cv::Mat* frame) = 0;
    virtual bool IsScene(const cv::Mat& frame) = 0;
  };

  bool IsBattleScene(const cv::Mat& frame);
  bool IsResultScene(const cv::Mat& frame);
  bool IsBlackoutScene(const cv::Mat& frame);
  bool IsWhiteoutScene(const cv::Mat& frame);

 private:
  Interface* battle_analyzer_;
  Interface* result_analyzer_;
  Interface* blackout_analyzer_;
  Interface* whiteout_analyzer_;
};
