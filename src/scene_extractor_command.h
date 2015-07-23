#pragma once

#include "command.h"

#include <stdint.h>

#include <string>

class SceneExtractorCommand : public Command {
 public:
  SceneExtractorCommand();
  virtual ~SceneExtractorCommand();

  virtual const char* GetCommandName() const {
    return "scene_extract";
  }
  virtual bool ProcessArgs(int argc, char** argv);
  virtual void PrintUsage(const char* myself);
  virtual void Run();

 private:
  static std::string MsecToString(int64_t msec);

  std::string video_path_;
  std::string battle_result_dir_;
  std::string title_result_dir_;
  bool is_nawabari_;
  bool is_debug_;
};
