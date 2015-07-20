#pragma once

class Command {
 public:
  Command() {}
  virtual ~Command() {}

  virtual const char* GetCommandName() const = 0;
  virtual bool ProcessArgs(int argc, char** argv) = 0;
  virtual void PrintUsage(const char* myself) = 0;
  virtual void Run() = 0;
};
