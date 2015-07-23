#pragma once

#include <opencv2/opencv.hpp>

#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/types.h>

#include "scene_analyzer/scene_analyzer.h"
#include "type.h"

class GameSceneExtractor {
 public:
  struct GameRegion {
    // Whole region of the game, inc. battle, result.
    TimeWindow game_msec;
    FrameWindow game_frame;

    TimeWindow title_msec;
    FrameWindow title_frame;

    TimeWindow battle_msec;
    FrameWindow battle_frame;

    TimeWindow result_msec;
    FrameWindow result_frame;
  };

  GameSceneExtractor(const std::string& video_path);
  ~GameSceneExtractor();

  bool FindNearestGameRegion(int64_t start_frame, GameRegion* out);

  int64_t video_frame_num() const {
    return video_frame_num_;
  }

  void GetImageAt(int64_t frame, cv::Mat* out);

 private:
  int64_t FindBattleFrameRoughly(int64_t start_frame);
  bool FindBattleStart(int64_t last_known_non_battle_frame,
                       int64_t known_battle_frame,
                       TimeWindow* out_time,
                       FrameWindow* out_frame);
  bool FindBattleEnd(int64_t known_battle_frame, TimeWindow* out_time,
                     FrameWindow* out_frame);
  bool FindResultEnd(int64_t battle_end_frame, TimeWindow* out_time,
                     FrameWindow* out_frame);
  int64_t FindEarliestBlackoutEndFrame(int64_t battle_start_frame);
  int64_t BlackoutLinearSearch(int64_t start_frame, int64_t end_frame);
  bool FindTitleRange(int64_t start_frame, int64_t end_frame,
                      TimeWindow* out_time, FrameWindow* out_frame);
  int64_t WhiteoutLinearSearch(int64_t start_frame, int64_t end_frame);


  const std::string video_path_;
  cv::VideoCapture* vc_;
  SceneAnalyzer* analyzer_;
  int64_t video_frame_num_;
  cv::Size video_size_;
};
