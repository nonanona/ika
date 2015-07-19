#pragma once

#include <opencv2/opencv.hpp>

class SceneAnalyzer {
 public:
  enum Scene {
    UNKNOWN = 0,
    BATTLE = 1,
    BLACKOUT = 2,
    RESULT = 3
  };
  SceneAnalyzer(const cv::Size& size);
  virtual ~SceneAnalyzer();

  class Interface {
   public:
    Interface() {}
    virtual ~Interface() {}
    virtual bool IsScene(const cv::Mat& frame) = 0;
  };

  Scene Process(const cv::Mat& frame);

 private:
  int fps_;
  long nframe_;
  int width_;
  int height_;

  Interface* battle_guesser_;
  Interface* result_guesser_;
  Interface* blackout_guesser_;

};
