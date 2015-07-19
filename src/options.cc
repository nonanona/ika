#include "options.h"

#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

Options::Options() : verbose_(false) {
}

Options::~Options() {
}

// static
Options* Options::GetInstance() {
  static Options* inst = NULL;
  if (!inst) {
    inst = new Options();
  }
  return inst;
}

// static
std::string Options::GetCmdOption(
    char** begin, char** end, const std::string& opt) {
  char** it = std::find(begin, end, opt);
  if (it != end && ++it != end) {
    return std::string(*it);
  }
  return std::string();
}

// static
bool Options::HasCmdOption(char** begin, char** end, const std::string& opt) {
  return std::find(begin, end, opt) != end;
}

// static
void Options::Init(int argc, char** argv) {
  GetInstance()->verbose_ = HasCmdOption(argv, argv + argc, "-v");
}
