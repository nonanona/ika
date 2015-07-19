#pragma once

#include <string>

class Options {
 public:
  ~Options();

  static void Init(int argc, char** argv);

  // True on verbose mode.
  static bool verbose() {
    return GetInstance()->verbose_;
  }

  // Utility functions.
  static std::string GetCmdOption(
      char** begin, char** end, const std::string& opt);
  static bool HasCmdOption(char** begin, char** end, const std::string& opt);
 private:
  Options();  // singleton

  static Options* GetInstance();

  bool verbose_;
};
