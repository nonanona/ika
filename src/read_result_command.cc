#include "read_result_command.h"

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
#include "template/stdout_output_handler.h"

ReadResultCommand::ReadResultCommand() {
}

ReadResultCommand::~ReadResultCommand() {
}

bool ReadResultCommand::ProcessArgs(int argc, char** argv) {
  image_path_ = GetCmdOption(argv + 1, argv + argc, "-i");
  return !image_path_.empty();
}

void ReadResultCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [image path] [--debug]\n",
         myself, GetCommandName());
}

void ReadResultCommand::Run() {
  ResultPageReader rpr;
  cv::Mat result_image = cv::imread(image_path_.c_str());
  rpr.LoadImage(result_image);

  OutputHandler* handler = new StdoutOutputHandler();

  handler->PushBattleId(0, TitlePageReader::UNKNOWN_RULE,
                        TitlePageReader::UNKNOWN_MAP);
  for (int i = 0; i < 8; ++i) {
    handler->PushBattleResult(
        0, i, i, rpr.ReadKillCount(i), rpr.ReadDeathCount(i),
        rpr.IsNawabari() ? rpr.ReadPaintPoint(i): -1,
        rpr.GetPlayerStatus(i));
  }
}
