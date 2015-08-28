#pragma once

#include <vector>
#include "template/output_handler.h"

class RedirectOutputHandler : public OutputHandler {
 public:
  RedirectOutputHandler();
  RedirectOutputHandler(std::string html_output_path,
                        bool also_output_to_stdout);

  virtual ~RedirectOutputHandler();

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
  std::vector<OutputHandler*> handlers_;
};
