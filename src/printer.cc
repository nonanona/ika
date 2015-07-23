#include "printer.h"

#include "ocr/result_page_reader.h"
#include "scene_analyzer/game_scene_extractor.h"

namespace printer {

namespace {
std::string MsecToString(int64_t msec) {
  const int hour_msec = 1000 * 60 * 60;
  const int min_msec = 1000 * 60;
  const int sec_msec = 1000;

  int hour = msec / hour_msec;
  msec -= hour * hour_msec;
  int min = msec / min_msec;
  msec -= min * min_msec;
  int sec = msec / sec_msec;
  msec -= sec * sec_msec;

  char buf[32];
  snprintf(buf, 32, "%02d:%02d:%02d.%03ld", hour, min, sec, msec);
  return std::string(buf);
}
}  // namespace

void PrintGameSceneSummary(const GameSceneExtractor::GameRegion region) {
  printf("  Scene Range:\n");
  printf("    Begin:    %s (%7ld frames)\n",
         MsecToString(region.game_msec.start).c_str(),
         region.game_frame.start);
  printf("    End:      %s (%7ld frames)\n",
         MsecToString(
             region.game_msec.start + region.game_msec.duration).c_str(),
         region.game_frame.start + region.game_frame.duration);
  printf("    Duration: %s (%7ld frames)\n",
         MsecToString(region.game_msec.duration).c_str(),
         region.game_frame.duration);
}

void PrintGameResult(const ResultPageReader& reader) {
  printf("  Play Result:\n");
  if (reader.is_nawabari()) {
    for (int i = 0; i < 8; ++i) {
      printf("    Player %d: %2d/%2d %4d Point\n",
             i, reader.ReadKillCount(i), reader.ReadDeathCount(i),
             reader.ReadPaintPoint(i));
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      printf("    Player %d: %2d/%2d\n",
             i, reader.ReadKillCount(i), reader.ReadDeathCount(i));
    }
  }
}

void PrintGameResultWithID(const ResultPageReader& reader,
                               int name_ids[8]) {
  printf("  Play Result:\n");
  if (reader.is_nawabari()) {
    for (int i = 0; i < 8; ++i) {
      printf("    Player %d (ID: %2d): %2d/%2d %4d Point\n",
             i, name_ids[i],
             reader.ReadKillCount(i), reader.ReadDeathCount(i),
             reader.ReadPaintPoint(i));
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      printf("    Player %d (ID: %2d): %2d/%2d\n",
             i, name_ids[i],
             reader.ReadKillCount(i), reader.ReadDeathCount(i));
    }
  }
}

}  // namespace printer
