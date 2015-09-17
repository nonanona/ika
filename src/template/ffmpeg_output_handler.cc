#include "template/ffmpeg_output_handler.h"

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

FfmpegOutputHandler::FfmpegOutputHandler(
    const std::string& out_file, const std::string& video_fname)
    : video_filename_(video_fname) {
  fp_ = fopen(out_file.c_str(), "w");
}

FfmpegOutputHandler::~FfmpegOutputHandler() {
  fclose(fp_);
}

void FfmpegOutputHandler::Initialize() {
  // intentionally empty.
}

void FfmpegOutputHandler::Finalize() {
  fflush(fp_);
}

void FfmpegOutputHandler::PushBattleId(int battle_id,
                                       TitlePageReader::Rule rule,
                                       TitlePageReader::Map map) {
  // intentionally empty.
}

void FfmpegOutputHandler::PushPlayerNameId(const cv::Mat& name_img,
                                           int name_id) {
  // intentionally empty.
}

void FfmpegOutputHandler::PushBattleSceneInfo(
    int battle_id, const GameSceneExtractor::GameRegion& region) {
  fprintf(fp_, "ffmpeg -ss %s -i %s -ss 0 -t %s -c:v copy -c:a copy"
         " -async 1 -strict 2 battle%04d.mp4\n",
         MsecToString(region.game_msec.start).c_str(),
         video_filename_.c_str(),
         MsecToString(region.game_msec.duration).c_str(),
         battle_id);

}

void FfmpegOutputHandler::PushBattleResult(int battle_id, int name_id,
                                           int ranked, int kill, int death,
                                           int paintpoint,
                                           ImageClipper::PlayerStatus status) {
  // intentionally empty.
}

void FfmpegOutputHandler::MaybeFlush() {
  fflush(fp_);
}
