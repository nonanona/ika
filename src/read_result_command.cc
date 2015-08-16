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
#include "printer.h"

ReadResultCommand::ReadResultCommand() {
}

ReadResultCommand::~ReadResultCommand() {
}

bool ReadResultCommand::ProcessArgs(int argc, char** argv) {
  image_path_ = GetCmdOption(argv + 1, argv + argc, "-i");
  is_debug_ = HasCmdOption(argv + 1, argv + argc, "--debug");
  return !image_path_.empty();
}

void ReadResultCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [image path] [--debug]\n",
         myself, GetCommandName());
}

void ReadResultCommand::Run() {
  ResultPageReader rpr;
  rpr.SetIsNawabari(true);
  cv::Mat result_image = cv::imread(image_path_.c_str());
  rpr.LoadImage(result_image);

  bool is_win = false;
  for (int i = 0; i < 8; ++i) {
    if (rpr.GetPlayerStatus(i) == ImageClipper::YOU) {
      is_win = i < 4;
      break;
    }
  }

  printf("Game:\n");
  printf("  Result: %s\n", is_win ? "YOU WIN" : "YOU LOSE");
  printer::PrintGameResult(rpr);

  if (is_debug_)
    rpr.ShowDebugImage(true);
}
