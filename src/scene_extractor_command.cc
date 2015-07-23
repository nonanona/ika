#include "scene_extractor_command.h"

#include <glog/logging.h>

#include "util/util.h"
#include "ocr/result_page_reader.h"
#include "ocr/name_tracker.h"
#include "scene_analyzer/game_scene_extractor.h"
#include "printer.h"

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
  NameTracker tracker;

  int battle_id = 0;
  int64_t frame = 0;
  while (true) {
    GameSceneExtractor::GameRegion region;
    if (!gse.FindNearestGameRegion(frame, &region))
      return;

    printf("Game %d:\n", battle_id);
    printer::PrintGameSceneSummary(region);

    cv::Mat result_image;
    const int64_t result_pos =
        region.result_frame.start + region.result_frame.duration / 2;
    gse.GetImageAt(result_pos, &result_image);
    rpr.LoadImage(result_image);

    int name_ids[8];
    for (int i = 0; i < 8; ++i) {
      name_ids[i] = tracker.GetNameId(rpr.GetNameImage(i));
    }

    printer::PrintGameResultWithID(rpr, name_ids);

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
