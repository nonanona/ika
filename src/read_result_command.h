#pragma once

#include "command.h"

#include <stdint.h>

#include <string>

class ReadResultCommand : public Command {
 public:
  ReadResultCommand();
  virtual ~ReadResultCommand();

  virtual const char* GetCommandName() const {
    return "read_result";
  }
  virtual bool ProcessArgs(int argc, char** argv);
  virtual void PrintUsage(const char* myself);
  virtual void Run();

 private:
  static std::string MsecToString(int64_t msec);

  std::string image_path_;
  bool is_debug_;
  bool is_nawabari_;
};
