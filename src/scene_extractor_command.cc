#include "scene_extractor_command.h"

#include <glog/logging.h>

#include "util/util.h"
#include "ocr/result_page_reader.h"
#include "scene_analyzer/game_scene_extractor.h"

SceneExtractorCommand::SceneExtractorCommand() {
}

SceneExtractorCommand::~SceneExtractorCommand() {
}

bool SceneExtractorCommand::ProcessArgs(int argc, char** argv) {
  video_path_ = GetCmdOption(argv + 1, argv + argc, "-i");
  is_nawabari_ = HasCmdOption(argv + 1, argv + argc, "--nawabari");
  is_debug_ = HasCmdOption(argv + 1, argv + argc, "--debug");
  battle_result_dir_ = GetCmdOption(argv + 1, argv + argc,
                                   "--battle-out-dir");
  return !video_path_.empty();
}

void SceneExtractorCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [video path] --battle-out-dir [output dir] \
         [--nawabari] [--debug]\n",
         myself, GetCommandName());
}

void SceneExtractorCommand::Run() {
  GameSceneExtractor gse(video_path_);
  ResultPageReader rpr(is_nawabari_);
  int battle_id = 0;
  int64_t frame = 0;
  while (true) {
    GameSceneExtractor::GameRegion region;
    if (!gse.FindNearestGameRegion(frame, &region))
      return;

    printf("Game %d:\n", battle_id);
    printf("  Scene Range:\n");
    printf("    Begin:    %s (%7ld frames)\n",
           MsecToString(region.game_msec.start).c_str(),
           region.game_frame.start);
    printf("    End:      %s (%7ld frames)\n",
           MsecToString(
               region.game_msec.start + region.game_msec.duration).c_str(),
           region.game_frame.start + region.game_frame.duration);
    printf("    Duration: %s (%7ld frames)\n",
           MsecToString(region.game_msec.duration).c_str(),
           region.game_frame.duration);

    cv::Mat result_image;
    const int64_t result_pos =
        region.result_frame.start + region.result_frame.duration / 2;
    gse.GetImageAt(result_pos, &result_image);
    rpr.LoadImage(result_image);

    printf("  Play Result:\n");
    if (is_nawabari_) {
      for (int i = 0; i < 8; ++i) {
        printf("    Player %d: %2d/%2d %4d Point\n",
               i, rpr.ReadKillCount(i), rpr.ReadDeathCount(i),
               rpr.ReadPaintPoint(i));
      }
    } else {
      for (int i = 0; i < 8; ++i) {
        printf("    Player %d: %2d/%2d\n",
               i, rpr.ReadKillCount(i), rpr.ReadDeathCount(i));
      }
    }

    if (!battle_result_dir_.empty()) {
      char buf[260];
      snprintf(buf, 64, "%s/battle%03d.png", battle_result_dir_.c_str(),
               battle_id);
      printf("Saving Battle results image to %s\n", buf);
      cv::imwrite(buf, result_image);
    }

    if (is_debug_)
      rpr.ShowDebugImage(true);

    battle_id++;
    frame = region.game_frame.start + region.game_frame.duration;
  }
}


std::string SceneExtractorCommand::MsecToString(int64_t msec) {
  const int hour_msec = 1000 * 60 * 60;
  const int min_msec = 1000 * 60;
  const int sec_msec = 1000;

  int hour = msec / hour_msec;
  msec -= hour * hour_msec;
  int min = msec / min_msec;
  msec -= min * min_msec;
  int sec = msec / sec_msec;
  msec -= sec * sec_msec;

  char buf[32];
  snprintf(buf, 32, "%02d:%02d:%02d.%03ld", hour, min, sec, msec);
  return std::string(buf);
}
