#pragma once

#include "ocr/result_page_reader.h"
#include "scene_analyzer/game_scene_extractor.h"

namespace printer {

void PrintGameSceneSummary(const GameSceneExtractor::GameRegion region);
void PrintGameResult(const ResultPageReader& reader);
void PrintGameResultWithID(const ResultPageReader& reader, int name_ids[8]);

}  // namespace printer

