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
    virtual bool IsScene(const cv::Mat& frame) = 0;
  };

  bool IsBattleScene(const cv::Mat& frame);
  bool IsResultScene(const cv::Mat& frame);
  bool IsBlackoutScene(const cv::Mat& frame);

 private:
  int fps_;
  long nframe_;
  int width_;
  int height_;

  Interface* battle_analyzer_;
  Interface* result_analyzer_;
  Interface* blackout_analyzer_;

};
