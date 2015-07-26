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
#include "printer.h"

SceneExtractorCommand::SceneExtractorCommand() {
}

SceneExtractorCommand::~SceneExtractorCommand() {
}

bool SceneExtractorCommand::ProcessArgs(int argc, char** argv) {
  video_path_ = GetCmdOption(argv + 1, argv + argc, "-i");
  is_debug_ = HasCmdOption(argv + 1, argv + argc, "--debug");
  battle_result_dir_ = GetCmdOption(argv + 1, argv + argc,
                                   "--battle-out-dir");
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

  int battle_id = 0;
  int64_t frame = 0;
  while (true) {
    GameSceneExtractor::GameRegion region;
    if (!gse.FindNearestGameRegion(frame, &region))
      return;

    std::vector<cv::Mat> image_sequence(4);
    int64_t title_frame =
        region.title_frame.start + region.title_frame.duration / 2;
    for (int i = 0; i < 4; ++i) {
      image_sequence[i] = cv::Mat();
      gse.GetImageAt(title_frame + (i - 2) * 8, &image_sequence[i]);
    }
    tpr.LoadImageSequence(image_sequence);

    rpr.SetIsNawabari(tpr.ReadRule() == TitlePageReader::NAWABARI);
    cv::Mat result_image;
    const int64_t result_pos =
        region.result_frame.start + region.result_frame.duration / 2;
    gse.GetImageAt(result_pos, &result_image);
    rpr.LoadImage(result_image);

    int name_ids[8];
    for (int i = 0; i < 8; ++i) {
      name_ids[i] = tracker.GetNameId(rpr.GetNameImage(i));
    }

    printf("Game %d:\n", battle_id);
    printf("  Rule: %s\n", tpr.GetRuleString(tpr.ReadRule()));
    printf("  Map : %s\n", tpr.GetMapString(tpr.ReadMap()));
    printf("  Result: %s\n", rpr.GetMyPosition() < 4 ? "YOU WIN" : "YOU LOSE");
    printer::PrintGameSceneSummary(region);
    printer::PrintGameResultWithID(rpr, name_ids);

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

    if (is_debug_)
      rpr.ShowDebugImage(true);

    battle_id++;
    frame = region.game_frame.start + region.game_frame.duration;
  }
}
