#pragma once

#include "command.h"

#include <stdint.h>

#include <string>

class LearningCommand : public Command {
 public:
  LearningCommand();
  virtual ~LearningCommand();

  virtual const char* GetCommandName() const {
    return "learning";
  }
  virtual bool ProcessArgs(int argc, char** argv);
  virtual void PrintUsage(const char* myself);
  virtual void Run();

 private:
  static std::string MsecToString(int64_t msec);
  double DoLearning(double criteria);

  std::string image_dir_;
  std::string out_file_;
  enum Mode {
    KILL_DEATH,
    PAINT_POINT,
    WEAPON
  } mode_;
  bool is_debug_;
  double xv_ratio_;
  int train_count_;
};
