#include "template/redirect_output_handler.h"

#include "ocr/result_page_reader.h"
#include "scene_analyzer/game_scene_extractor.h"
#include "template/stdout_output_handler.h"


RedirectOutputHandler::RedirectOutputHandler() {
  handlers_.push_back(new StdoutOutputHandler());
}

RedirectOutputHandler::RedirectOutputHandler(
    std::string html_output_path, bool also_output_to_stdout) {
  if (also_output_to_stdout)
    handlers_.push_back(new StdoutOutputHandler());
  // TODO add html output handler.
}

RedirectOutputHandler::~RedirectOutputHandler() {
  for (size_t i = 0; i < handlers_.size(); ++i) {
    delete handlers_[i];
  }
}

void RedirectOutputHandler::AddHandler(OutputHandler* handler) {
  handlers_.push_back(handler);
}

void RedirectOutputHandler::Initialize() {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->Initialize();
  }
}

void RedirectOutputHandler::Finalize() {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->Finalize();
  }
}

void RedirectOutputHandler::PushBattleId(int battle_id,
                                         TitlePageReader::Rule rule,
                                         TitlePageReader::Map map) {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->PushBattleId(battle_id, rule, map);
  }
}

void RedirectOutputHandler::PushPlayerNameId(const cv::Mat& name_img,
                                             int name_id) {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->PushPlayerNameId(name_img, name_id);
  }
}

void RedirectOutputHandler::PushBattleSceneInfo(
    int battle_id, const GameSceneExtractor::GameRegion& region) {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->PushBattleSceneInfo(battle_id, region);
  }
}

void RedirectOutputHandler::PushBattleResult(
    int battle_id, int name_id, int ranked, int kill, int death,
    int paintpoint, ImageClipper::PlayerStatus status) {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->PushBattleResult(battle_id, name_id, ranked, kill, death,
                                   paintpoint, status);
  }
}

void RedirectOutputHandler::MaybeFlush() {
  for (int i = 0; i < handlers_.size(); ++i) {
    handlers_[i]->MaybeFlush();
  }
}
