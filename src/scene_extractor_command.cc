#include "scene_extractor_command.h"

#include <glog/logging.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util/util.h"
#include "util/debugger.h"
#include "ocr/result_page_reader.h"
#include "ocr/title_page_reader.h"
#include "ocr/name_tracker.h"
#include "scene_analyzer/game_scene_extractor.h"
#include "template/output_handler.h"
#include "template/redirect_output_handler.h"
#include "template/ffmpeg_output_handler.h"

SceneExtractorCommand::SceneExtractorCommand() {
}

SceneExtractorCommand::~SceneExtractorCommand() {
}

bool SceneExtractorCommand::ProcessArgs(int argc, char** argv) {
  video_path_ = GetCmdOption(argv + 1, argv + argc, "-i");
  battle_result_dir_ = GetCmdOption(argv + 1, argv + argc,
                                   "--battle-out-dir");
  ffmpeg_output_file_ = GetCmdOption(argv + 1, argv + argc, "--ffmpeg");
  return !video_path_.empty();
}

void SceneExtractorCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [video path] --battle-out-dir [output dir] \
         [--debug]\n",
         myself, GetCommandName());
}

void SceneExtractorCommand::Run() {
  GameSceneExtractor gse(video_path_);
  ResultPageReader rpr;
  TitlePageReader tpr;
  NameTracker tracker;

  RedirectOutputHandler* handler = new RedirectOutputHandler();

  if (!ffmpeg_output_file_.empty()) {
      handler->AddHandler(
          new FfmpegOutputHandler(ffmpeg_output_file_, video_path_));
  }

  int battle_id = 0;
  int64_t frame = 0;
  while (true) {
    GameSceneExtractor::GameRegion region;
    if (!gse.FindNearestGameRegion(frame, &region))
      return;

    cv::Mat title_image;
    int64_t title_frame =
        region.title_frame.start + region.title_frame.duration / 2;
    gse.GetImageAt(title_frame, &title_image);
    tpr.LoadImage(title_image);

    cv::Mat result_image;
    const int64_t result_pos =
        region.result_frame.start + region.result_frame.duration / 2;
    gse.GetImageAt(result_pos, &result_image);
    rpr.LoadImage(result_image);

    handler->PushBattleId(battle_id, tpr.ReadRule(), tpr.ReadMap());
    handler->PushBattleSceneInfo(battle_id, region);

    int name_ids[8];
    int my_position;
    for (int i = 0; i < 8; ++i) {
      ImageClipper::PlayerStatus player_status = rpr.GetPlayerStatus(i);
      if (player_status == ImageClipper::VACANCY)
        continue;
      else if (player_status == ImageClipper::YOU)
        my_position = i;
      name_ids[i] = tracker.GetNameId(rpr.GetNameImage(i));
      handler->PushPlayerNameId(rpr.GetNameImage(i), name_ids[i]);
      handler->PushBattleResult(
          battle_id, name_ids[i], i, rpr.ReadKillCount(i),
          rpr.ReadDeathCount(i),
          rpr.IsNawabari() ? rpr.ReadPaintPoint(i): -1,
          rpr.GetPlayerStatus(i));
    }

    if (!battle_result_dir_.empty()) {
      char buf[260];
      for (int i = 0; i < 10000; ++i) {
        snprintf(buf, 64, "%s/battle%03d.png", battle_result_dir_.c_str(),
                 battle_id);
        int fd = open(buf, O_RDWR);
        if (fd == -1) {
          break;
        } else {
          close(fd);
        }
      }
      printf("Saving Battle results image to %s\n", buf);
      cv::imwrite(buf, result_image);
    }

    handler->MaybeFlush();

    battle_id++;
    frame = region.game_frame.start + region.game_frame.duration;
  }
}
