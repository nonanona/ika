#pragma once 

#include "template/output_handler.h"

class FfmpegOutputHandler : public OutputHandler {
 public:
  FfmpegOutputHandler(const std::string& out_file,
                      const std::string& video_path);
  virtual ~FfmpegOutputHandler();

  virtual void Initialize();
  virtual void Finalize();
  virtual void PushBattleId(int battle_id, TitlePageReader::Rule,
                            TitlePageReader::Map);

  virtual void PushPlayerNameId(const cv::Mat& name_img, int name_id);
  virtual void PushBattleSceneInfo(
      int battle_id, const GameSceneExtractor::GameRegion& region);
  virtual void PushBattleResult(int battle_id, int name_id, int ranked,
                                int kill, int death, int paintpoint,
                                ImageClipper::PlayerStatus status);
  virtual void MaybeFlush();

 private:
  FILE* fp_;
  std::string video_filename_;
};
