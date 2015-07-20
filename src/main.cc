#include <glog/logging.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

#include "command.h"
#include "scene_extractor_command.h"


int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  Command* commands[] = {
    new SceneExtractorCommand()
  };

  const std::string command(argc == 1 ? "" : argv[1]);
  if (command.empty()) {
    printf("Usage: %s [command]\n", argv[0]);
    printf("Available commands\n");
    for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); ++i) {
      printf("  %s\n", commands[i]->GetCommandName());
    }
    exit(1);
  }


  for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); ++i) {
    if (command != commands[i]->GetCommandName())
      continue;
    if (!commands[i]->ProcessArgs(argc, argv)) {
      commands[i]->PrintUsage(argv[0]);
      exit(1);
    }

    commands[i]->Run();
  }

  google::ShutdownGoogleLogging();
}
