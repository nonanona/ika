#include "template/stdout_output_handler.h"

#include "ocr/result_page_reader.h"
#include "scene_analyzer/game_scene_extractor.h"

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

StdoutOutputHandler::StdoutOutputHandler() {
}

StdoutOutputHandler::~StdoutOutputHandler() {
}

void StdoutOutputHandler::Initialize() {
  // intentionally empty.
}

void StdoutOutputHandler::Finalize() {
  fflush(stdout);
}

void StdoutOutputHandler::PushBattleId(int battle_id,
                                       TitlePageReader::Rule rule,
                                       TitlePageReader::Map map) {
  printf("Game %d:\n", battle_id);
  printf("  Rule: %s\n", TitlePageReader::GetRuleString(rule));
  printf("  Map : %s\n", TitlePageReader::GetMapString(map));
}

void StdoutOutputHandler::PushPlayerNameId(const cv::Mat& name_img,
                                           int name_id) {
  // intentionally empty.
}

void StdoutOutputHandler::PushBattleSceneInfo(
    int battle_id, const GameSceneExtractor::GameRegion& region) {
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

void StdoutOutputHandler::PushBattleResult(int battle_id, int name_id,
                                           int ranked, int kill, int death,
                                           int paintpoint,
                                           ImageClipper::PlayerStatus status) {
  if (ranked == 0) {
    printf("  Play Result:\n");
  }

  if (status == ImageClipper::VACANCY) {
    printf("    Player -\n");
    return;
  }
  printf("    Player %d (ID: %2d): %2d/%2d", ranked, name_id, kill, death);
  if (paintpoint >= 0)
    printf(" %4d Point", paintpoint);
  if (status == ImageClipper::YOU)
    printf(" <- You");
  printf("\n");
}

void StdoutOutputHandler::MaybeFlush() {
  fflush(stdout);
}
