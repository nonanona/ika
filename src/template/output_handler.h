#pragma once

#include "ocr/title_page_reader.h"
#include "ocr/image_clipper.h"
#include "scene_analyzer//game_scene_extractor.h"


class OutputHandler {
 public:
  OutputHandler() {}
  virtual ~OutputHandler() {}

  virtual void Initialize() = 0;
  virtual void Finalize() = 0;

  virtual void PushBattleId(int battle_id, TitlePageReader::Rule,
                            TitlePageReader::Map) = 0;
  virtual void PushPlayerNameId(const cv::Mat& name_img, int name_id) = 0;
  virtual void PushBattleSceneInfo(
      int battle_id, const GameSceneExtractor::GameRegion& region) = 0;
  virtual void PushBattleResult(int battle_id, int name_id, int ranked,
                                int kill, int death, int paintpoint,
                                ImageClipper::PlayerStatus status) = 0;
  virtual void MaybeFlush() = 0;
};
