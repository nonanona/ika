#pragma once

#include "job.h"

#include <stdint.h>

#include <string>

class SceneExtractorJob : public Job {
 public:
  SceneExtractorJob();
  virtual ~SceneExtractorJob();

  virtual const char* GetCommandName() const {
    return "scene_extract";
  }
  virtual bool ProcessArgs(int argc, char** argv);
  virtual void PrintUsage(const char* myself);
  virtual void Run();

 private:
  static std::string MsecToString(int64_t msec);

  std::string video_path_;
  bool is_nawabari_;
  bool is_debug_;
};
