#pragma once

#include "command.h"

#include <stdint.h>

#include <string>

class ComponentExtractCommand : public Command {
 public:
  ComponentExtractCommand();
  virtual ~ComponentExtractCommand();

  virtual const char* GetCommandName() const {
    return "component_extract";
  }
  virtual bool ProcessArgs(int argc, char** argv);
  virtual void PrintUsage(const char* myself);
  virtual void Run();

 private:
  void SaveToFile(const std::string& path);
  int GetNearestFileIndex(int index, const std::string& dir, std::string* out);

  int name_index_;
  int weapon_index_;
  int kill_death_index_;
  int paintpoint_index_;

  std::string image_path_;
  std::string output_dir_;
  bool is_nawabari_;
  bool is_debug_;
  bool is_overwrite_;
  bool predict_;
  bool is_camera_;
};
