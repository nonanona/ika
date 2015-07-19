#include <glog/logging.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

#include "job.h"
#include "options.h"
#include "scene_extractor_job.h"


int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  Job* jobs[] = {
    new SceneExtractorJob()
  };

  const std::string command(argc == 1 ? "" : argv[1]);
  if (command.empty()) {
    printf("Usage: %s [command]\n", argv[0]);
    printf("Available commands\n");
    for (int i = 0; i < sizeof(jobs)/sizeof(jobs[0]); ++i) {
      printf("  %s\n", jobs[i]->GetCommandName());
    }
    exit(1);
  }


  for (int i = 0; i < sizeof(jobs)/sizeof(jobs[0]); ++i) {
    if (command != jobs[i]->GetCommandName())
      continue;
    if (!jobs[i]->ProcessArgs(argc, argv)) {
      jobs[i]->PrintUsage(argv[0]);
      exit(1);
    }

    jobs[i]->Run();
  }

  google::ShutdownGoogleLogging();
}
