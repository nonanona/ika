#pragma once

#include "command.h"

#include <stdint.h>

#include <string>

class CameraCaptureCommand : public Command {
 public:
  CameraCaptureCommand();
  virtual ~CameraCaptureCommand();

  virtual const char* GetCommandName() const {
    return "camera_capture";
  }
  virtual bool ProcessArgs(int argc, char** argv);
  virtual void PrintUsage(const char* myself);
  virtual void Run();

 private:
  static std::string MsecToString(int64_t msec);
};
