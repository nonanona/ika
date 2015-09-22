#include <opencv2/opencv.hpp>

#include <glog/logging.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#include "scene_analyzer/game_scene_extractor.h"
#include "scene_analyzer/scene_analyzer.h"

GameSceneExtractor::GameSceneExtractor(const std::string& video_path)
  : video_path_(video_path), vc_(new cv::VideoCapture(video_path)) {
  if (!vc_->isOpened())
    LOG(FATAL) << "Failed to open Video File: " << video_path;
  video_frame_num_ = static_cast<int64_t>(vc_->get(CV_CAP_PROP_FRAME_COUNT));
  video_size_ = cv::Size(
      static_cast<int64_t>(vc_->get(CV_CAP_PROP_FRAME_WIDTH)),
      static_cast<int64_t>(vc_->get(CV_CAP_PROP_FRAME_HEIGHT)));
  analyzer_ = new SceneAnalyzer(video_size_);
}

GameSceneExtractor::~GameSceneExtractor() {
  delete vc_;
  delete analyzer_;
}

int64_t GameSceneExtractor::FindBattleFrameRoughly(int64_t start_frame) {
  cv::Mat frame_img;

  vc_->set(CV_CAP_PROP_POS_FRAMES, start_frame);
  int64_t target_msec = (int64_t)vc_->get(CV_CAP_PROP_POS_MSEC);
  // Search the battle scene by skipping 30 sec.
  while (true) {
    vc_->set(CV_CAP_PROP_POS_MSEC, target_msec);
    vc_->read(frame_img);
    if (frame_img.empty()) {
      LOG(INFO) << "Failed to obtain frame image at " << target_msec;
      return -1;
    }

    if (analyzer_->IsBattleScene(frame_img)) {
      VLOG(3) << "Find battle field roughly at " << target_msec;
      return vc_->get(CV_CAP_PROP_POS_FRAMES);
    } else {
      VLOG(3) << "Not a battle field. Skipping 30 sec";
      target_msec += 30 * 1000;
    }
  }
}

bool GameSceneExtractor::FindBattleStart(int64_t last_known_non_battle_frame,
                                         int64_t known_battle_frame,
                                         TimeWindow* out_time,
                                         FrameWindow* out_frame) {
  int64_t bisect_start = last_known_non_battle_frame;
  int64_t bisect_end = known_battle_frame;

  VLOG(3) << "Start Battle Start bisect: initial range "
      << "(" << bisect_start << ", " << bisect_end << ")";
  cv::Mat frame_img;
  while (true) {
    int64_t trial = (bisect_start + bisect_end) / 2;
    vc_->set(CV_CAP_PROP_POS_FRAMES, trial);

    if (trial == bisect_start || trial == bisect_end) {
      VLOG(3) << "Finish bisect: " << trial;
      out_time->start = vc_->get(CV_CAP_PROP_POS_MSEC);
      out_frame->start = vc_->get(CV_CAP_PROP_POS_FRAMES);
      return true;
    }

    int msec = vc_->get(CV_CAP_PROP_POS_MSEC);
    vc_->read(frame_img);
    if (frame_img.empty()) {
      LOG(ERROR) << "Failed to obtain frame image at " << msec;
      return false;
    }
    if (analyzer_->IsBattleScene(frame_img)) {
      VLOG(3) << "Updatiung bisect range: "
          << "(" << bisect_start << ", " << bisect_end << ") ->"
          << "(" << bisect_start << ", " << trial << ")";
      bisect_end = trial;
    } else {
      VLOG(3) << "Updatiung bisect range: "
          << "(" << bisect_start << ", " << bisect_end << ") ->"
          << "(" << trial << ", " << bisect_end << ")";
      bisect_start = trial;
    }
  }
}

bool GameSceneExtractor::FindBattleEnd(int64_t current_battle_frame,
                                       TimeWindow* out_time,
                                       FrameWindow* out_frame) {
  cv::Mat frame_img;

  // First, find the non battle scene roughly.
  vc_->set(CV_CAP_PROP_POS_FRAMES, current_battle_frame);
  int64_t target_msec = (int64_t)vc_->get(CV_CAP_PROP_POS_MSEC);
  // Search the non battle scene by skipping 30 sec.
  while (true) {
    vc_->set(CV_CAP_PROP_POS_MSEC, target_msec);
    vc_->read(frame_img);
    if (frame_img.empty()) {
      LOG(INFO) << "Failed to obtain frame image at " << target_msec;
      return false;
    }

    if (analyzer_->IsBattleScene(frame_img)) {
      VLOG(3) << "Still battle field. Skipping 30 sec";
      target_msec += 30 * 1000;
    } else {
      VLOG(3) << "Find non battle field roughly at " << target_msec;
      VLOG(3) << "check 3sec after since blackout may happen in battle.";
      vc_->set(CV_CAP_PROP_POS_MSEC, target_msec + 3000);
      vc_->read(frame_img);

      if (analyzer_->IsBattleScene(frame_img)) {
        VLOG(3) << "Yeah, still in battle, seach next 30 sec.";
        target_msec += 30 * 1000;
        continue;
      }
      break;
    }
  }

  // Second, search the battle end position by bisect.
  int64_t bisect_start_msec = target_msec - 30 * 1000;
  int64_t bisect_end_msec = target_msec;

  VLOG(3) << "Start bisect for battle end: initial range "
      << "(" << bisect_start_msec << ", " << bisect_end_msec << ")";
  while (true) {
    int64_t trial = (bisect_start_msec + bisect_end_msec) / 2;

    if (trial == bisect_start_msec || trial == bisect_end_msec) {
      VLOG(3) << "Finish bisect: " << trial;
      vc_->set(CV_CAP_PROP_POS_MSEC, trial);
      out_time->duration = vc_->get(CV_CAP_PROP_POS_MSEC) - out_time->start;
      out_frame->duration = vc_->get(CV_CAP_PROP_POS_FRAMES) - out_frame->start;
      return true;
    }

    vc_->set(CV_CAP_PROP_POS_MSEC, trial);
    vc_->read(frame_img);
    if (frame_img.empty()) {
      LOG(ERROR) << "Failed to obtain frame image at " << target_msec;
      return false;
    }
    if (!analyzer_->IsBattleScene(frame_img)) {
      VLOG(3) << "Updatiung bisect range: "
          << "(" << bisect_start_msec << ", " << bisect_end_msec << ") ->"
          << "(" << bisect_start_msec << ", " << trial << ")";
      bisect_end_msec = trial;
    } else {
      VLOG(3) << "Updatiung bisect range: "
          << "(" << bisect_start_msec << ", " << bisect_end_msec << ") ->"
          << "(" << bisect_start_msec << ", " << trial << ")";
      bisect_start_msec = trial;
    }
  }
  return false;
}

bool GameSceneExtractor::FindResultEnd(
    int64_t battle_end_frame, TimeWindow* out_time, FrameWindow* out_frame) {
  cv::Mat frame_img;

  bool result_started = false;
  int non_result_count = 0;
  int64_t res_msec = 0;

  // After battle end, there is 10 sec judge-kun performance. Skip it.
  vc_->set(CV_CAP_PROP_POS_FRAMES, battle_end_frame);
  int64_t battle_end_msec = vc_->get(CV_CAP_PROP_POS_MSEC);
  vc_->set(CV_CAP_PROP_POS_MSEC, battle_end_msec + 10 * 1000);

  while (true) {
    int64_t cur_msec = vc_->get(CV_CAP_PROP_POS_MSEC);
    if (cur_msec - battle_end_msec > 60000) {
      // Give up if can not find the result end until 1 min.
      return false;
    }
    vc_->read(frame_img);
    if (frame_img.empty()) {
      LOG(ERROR) << "Failed to obtain frame image at "
          << vc_->get(CV_CAP_PROP_POS_FRAMES);
      return false;
    }

    // Due to un-reliable scene analyzer, we doesn't regard non result scene as
    // the end of result scene. Check 10 proceeding frame and if all of them are
    // non result scene, we finalizes the end of result point.
    if (analyzer_->IsResultScene(frame_img)) {
      if (!result_started) {
        out_time->start = vc_->get(CV_CAP_PROP_POS_MSEC);
        out_frame->start = vc_->get(CV_CAP_PROP_POS_FRAMES);
      }
      result_started = true;
      non_result_count = 0;
    } else if (result_started) {
      if (non_result_count == 0) {
        res_msec = vc_->get(CV_CAP_PROP_POS_MSEC);
        non_result_count++;
      } else if (non_result_count == 10) {
        res_msec -= 1000;  // hack. Udemae window is comming from bottom.
        vc_->set(CV_CAP_PROP_POS_MSEC, res_msec);
        out_time->duration = vc_->get(CV_CAP_PROP_POS_MSEC) - out_time->start;
        out_frame->duration =
            vc_->get(CV_CAP_PROP_POS_FRAMES) - out_frame->start;
        return true;
      } else {
        non_result_count++;
      }
    }
  }
  return false;
}

int64_t GameSceneExtractor::BlackoutLinearSearch(
    int64_t start_frame, int64_t end_frame) {
  VLOG(3) << "LinearSearch Blackout Scene: "
      << " (" << start_frame << ", " << end_frame << ")";
  cv::Mat frame_img;
  vc_->set(CV_CAP_PROP_POS_FRAMES, start_frame);
  while (true) {
    int64_t frame = vc_->get(CV_CAP_PROP_POS_FRAMES);
    if (frame >= end_frame)
      return -1;

    vc_->read(frame_img);
    if (frame_img.empty())
      return -1;

    if (analyzer_->IsBlackoutScene(frame_img))
      return vc_->get(CV_CAP_PROP_POS_FRAMES);
  }
  return -1;
}

bool GameSceneExtractor::FindTitleRange(
    int64_t start_frame, int64_t end_frame,
    TimeWindow* out_time, FrameWindow* out_frame) {
  VLOG(3) << "LinearSearch Whiteout Scene: "
      << " (" << start_frame << ", " << end_frame << ")";
  int64_t end_title_frame = WhiteoutLinearSearch(start_frame, end_frame);
  if (end_title_frame == -1)
    return false;

  out_frame->start = start_frame;
  out_frame->duration = end_title_frame - start_frame;

  vc_->set(CV_CAP_PROP_POS_FRAMES, start_frame);
  out_time->start = vc_->get(CV_CAP_PROP_POS_MSEC);
  vc_->set(CV_CAP_PROP_POS_FRAMES, end_title_frame);
  out_time->duration = vc_->get(CV_CAP_PROP_POS_MSEC) - out_time->start;
  return true;
}

int64_t GameSceneExtractor::WhiteoutLinearSearch(int64_t start_frame,
                                                 int64_t end_frame) {
  VLOG(3) << "LinearSearch Whiteout Scene: "
      << " (" << start_frame << ", " << end_frame << ")";
  cv::Mat frame_img;
  vc_->set(CV_CAP_PROP_POS_FRAMES, start_frame);
  while (true) {
    int64_t frame = vc_->get(CV_CAP_PROP_POS_FRAMES);
    if (frame >= end_frame)
      return -1;

    vc_->read(frame_img);
    if (frame_img.empty())
      return -1;

    if (analyzer_->IsWhiteoutScene(frame_img))
      return vc_->get(CV_CAP_PROP_POS_FRAMES);
  }
  return -1;
}

int64_t GameSceneExtractor::FindEarliestBlackoutEndFrame(
    int64_t battle_start_frame) {
  // Due to performance issue, unable to increment backward.
  // Check 10 sec each.

  vc_->set(CV_CAP_PROP_POS_FRAMES, battle_start_frame);
  int64_t battle_start_frame_msec = vc_->get(CV_CAP_PROP_POS_MSEC);

  for (int i = 0; i < 6; ++i) {
    int64_t window_start_ms = battle_start_frame_msec - (i + 1) * 10 * 1000;
    int64_t window_end_ms = battle_start_frame_msec - i * 10 * 1000;

    vc_->set(CV_CAP_PROP_POS_MSEC, window_start_ms);
    int64_t start_frame = vc_->get(CV_CAP_PROP_POS_FRAMES);
    vc_->set(CV_CAP_PROP_POS_MSEC, window_end_ms);
    int64_t end_frame = vc_->get(CV_CAP_PROP_POS_FRAMES);


    int64_t r = BlackoutLinearSearch(start_frame, end_frame);
    if (r == -1)
      continue;

    cv::Mat frame_img;
    while (true) {
      vc_->read(frame_img);
      if (frame_img.empty())
        return -1;
      if (!analyzer_->IsBlackoutScene(frame_img))
        return vc_->get(CV_CAP_PROP_POS_FRAMES);
    }
    return -1;
  }

  return -1;
}

bool GameSceneExtractor::FindNearestGameRegion(
    int64_t start_frame, GameSceneExtractor::GameRegion* out) {

  int64_t current_frame = start_frame;

  int64_t battle_frame = FindBattleFrameRoughly(current_frame);
  if (battle_frame == -1) {
    LOG(INFO) << "Failed to find battle frame.";
    return false;
  }

  if (battle_frame == start_frame) {
    LOG(ERROR) << "Currently not suppoted the video starts from battle scene";
    return false;
  }

  if (!FindBattleStart(current_frame, battle_frame, &out->battle_msec,
                       &out->battle_frame)) {
    LOG(ERROR) << "Failed to find battle start region.";
    return false;
  }

  if (!FindBattleEnd(battle_frame, &out->battle_msec,
                     &out->battle_frame)) {
    LOG(WARNING) << "Failed to find end of battle scene.";
    return false;
  }

  if (!FindResultEnd(out->battle_frame.start + out->battle_frame.duration,
                     &out->result_msec, &out->result_frame)) {
    return FindNearestGameRegion(
        out->battle_frame.start + out->battle_frame.duration + 1,
        out);
  }

  int64_t blackout_end_frame = FindEarliestBlackoutEndFrame(
      out->battle_frame.start);
  if (blackout_end_frame == -1) {
    LOG(ERROR) << "Failed to find blackout battle enter position.";
    return false;
  }

  if (!FindTitleRange(blackout_end_frame, out->battle_frame.start,
                      &out->title_msec, &out->title_frame)) {
    LOG(ERROR) << "Failed to find battle title region.";
    return false;
  }

  vc_->set(CV_CAP_PROP_POS_FRAMES, blackout_end_frame);

  out->game_msec.start = vc_->get(CV_CAP_PROP_POS_MSEC);
  out->game_msec.duration =
      out->result_msec.start + out->result_msec.duration -
      out->game_msec.start;
  out->game_frame.start = vc_->get(CV_CAP_PROP_POS_FRAMES);
  out->game_frame.duration =
      out->result_frame.start + out->result_frame.duration -
      out->game_frame.start;
  return true;
}

void GameSceneExtractor::GetImageAt(int64_t frame, cv::Mat* out) {
  vc_->set(CV_CAP_PROP_POS_FRAMES, frame);
  vc_->read(*out);
}
