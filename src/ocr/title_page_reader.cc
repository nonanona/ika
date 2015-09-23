#include "ocr/title_page_reader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <glog/logging.h>
#include "util/debugger.h"
#include "util/util.h"

TitlePageReader::TitlePageReader()
    : map_(URCHIN_UNDERPASS),
      rule_(NAWABARI),
      is_initialized_(false) {
}

void TitlePageReader::initialize(const cv::Size& size, bool is_camera) {
  std::string res_dir;
  if (size.width == 1920 && size.height == 1080) {
    initialize1080();
  } else if (size.width == 1280 && size.height == 720) {
    initialize720();
  } else {
    LOG(ERROR) << "Unsupported image size: "
        << size.width << "x" << size.height;
    abort();
  }
  is_initialized_ = true;
  is_camera_ = is_camera;
}

void TitlePageReader::initialize720() {
#define LOAD_MAP_IMAGE(key, rect, fname) \
  maps_[key] = cv::imread("res720/map/" fname ".png", 0); \
  map_rects_[key] = rect; \
  if (maps_[key].empty()) { \
    LOG(ERROR) << fname << " not found"; \
    abort(); \
  }
  LOAD_MAP_IMAGE(URCHIN_UNDERPASS, cv::Rect(881, 579, 342, 61),
                 "urchin_underpass");
  LOAD_MAP_IMAGE(WALLEYE_WAREHOUSE, cv::Rect(956, 581, 268, 59),
                 "walleye_warehouse");
  LOAD_MAP_IMAGE(SALTSPRAY_RIG, cv::Rect(970, 593, 252, 47),
                 "saltspray_rig");
  LOAD_MAP_IMAGE(AROWANA_MALL, cv::Rect(940, 595, 284, 45), "arowana_mall");
  LOAD_MAP_IMAGE(BLACKBELLY_SKATEPARK, cv::Rect(960, 588, 264, 52),
                 "blackbelly_skatepark");
  LOAD_MAP_IMAGE(PORT_MACKEREL, cv::Rect(990,595, 234, 45), "port_mackerel");
  LOAD_MAP_IMAGE(KELP_DOME, cv::Rect(1003, 581, 220, 59), "kelp_dome");
  LOAD_MAP_IMAGE(BLUEFIN_DEPOT, cv::Rect(969,587, 255, 53), "bluefin_depot");
  LOAD_MAP_IMAGE(MORAY_TOWERS, cv::Rect(856, 581, 367, 59), "moray_towers");
  LOAD_MAP_IMAGE(CAMP_TRIGGERFISH, cv::Rect(863, 580, 360, 60),
                 "camp_triggerfish");
  LOAD_MAP_IMAGE(HIRAME, cv::Rect(912, 583, 311, 57), "hirame");
  LOAD_MAP_IMAGE(MASABA, cv::Rect(886, 588, 336, 54), "masaba");
#undef LOAD_MAP_IMAGE

#define LOAD_RULE_IMAGE(key, rect, fname) \
  rules_[key] = cv::imread("res720/rule/" fname ".png", 0); \
  rule_rects_[key] = rect
  LOAD_RULE_IMAGE(NAWABARI, cv::Rect(487, 254, 303, 52), "nawabari");
  LOAD_RULE_IMAGE(AREA, cv::Rect(534, 251, 206, 55), "area");
  LOAD_RULE_IMAGE(YAGURA, cv::Rect(535, 251, 207, 55), "yagura");
  LOAD_RULE_IMAGE(HOKO, cv::Rect(488, 251, 300, 55), "hoko");
#undef LOAD_RULE_IMAGE
}

void TitlePageReader::initialize1080() {
  LOG(ERROR) << "Not implemented yet";
  abort();
}

TitlePageReader::~TitlePageReader() {
}

void TitlePageReader::LoadImage(const cv::Mat& image, bool is_camera) {
  if (!is_initialized_)
    initialize(image.size(), is_camera);

  cv::Mat img;
  ExtractWhite(image, &img);
  
  map_ = UNKNOWN_MAP;
  double best_map_score = 1.00;
  for (int i = 1; i < NUM_OF_MAPS; ++i) {
    cv::Rect r = map_rects_[i];

    if (is_camera_) {
      r.x += 2; r.y += 2;
    }

    cv::Mat clipped = img(r);
    double candidate = ImageDiff(clipped, maps_[i]);
    if (candidate < 0.1 && candidate < best_map_score) {
      best_map_score = candidate;
      map_ = (Map)i;
    }
  }

  rule_ = UNKNOWN_RULE;
  double best_rule_score = 1.00;
  for (int i = 1; i < NUM_OF_RULES; ++i) {
    cv::Mat clipped = img(rule_rects_[i]);
    double candidate = ImageDiff(clipped, rules_[i]);
    if (candidate < 0.1 && candidate < best_rule_score) {
      best_rule_score = candidate;
      rule_ = (Rule)i;
    }
  }
}

TitlePageReader::Rule TitlePageReader::ReadRule() const {
  return rule_;
}

TitlePageReader::Map TitlePageReader::ReadMap() const {
  return map_;
}

// static
const char* TitlePageReader::GetRuleString(TitlePageReader::Rule rule) {
  switch (rule) {
    case UNKNOWN_RULE:
      return "UNKNOWN";
    case NAWABARI:
      return "\xE3\x83\x8A\xE3\x83\xAF\xE3\x83\x90\xE3\x83\xAA\xE3\x83\x90"
          "\xE3\x83\x88\xE3\x83\xAB";
    case AREA:
      return "\xE3\x82\xAC\xE3\x83\x81\xE3\x82\xA8\xE3\x83\xAA\xE3\x82\xA2";
    case YAGURA:
      return "\xE3\x82\xAC\xE3\x83\x81\xE3\x83\xA4\xE3\x82\xB0\xE3\x83\xA9";
    case HOKO:
      return "\xE3\x82\xAC\xE3\x83\x81\xE3\x83\x9B\xE3\x82\xB3";
    default:
      abort();
  }
}

// static
const char* TitlePageReader::GetMapString(TitlePageReader::Map map) {
  switch (map) {
    case UNKNOWN_MAP:
      return "UNKNOWN";
    case URCHIN_UNDERPASS:
      return "\xE3\x83\x87\xE3\x82\xAB\xE3\x83\xA9\xE3\x82\xA4\xE3\x83\xB3"
          "\xE9\xAB\x98\xE6\x9E\xB6\xE4\xB8\x8B";
    case WALLEYE_WAREHOUSE:
      return "\xE3\x83\x8F\xE3\x82\xB3\xE3\x83\x95\xE3\x82\xB0\xE5\x80\x89"
          "\xE5\xBA\xAB";
    case SALTSPRAY_RIG:
      return "\xE3\x82\xB7\xE3\x82\xAA\xE3\x83\x8E\xE3\x83\xA1\xE6\xB2\xB9"
          "\xE7\x94\xB0";
    case AROWANA_MALL:
      return "\xE3\x82\xA2\xE3\x83\xAD\xE3\x83\xAF\xE3\x83\x8A\xE3\x83\xA2"
          "\xE3\x83\xBC\xE3\x83\xAB";
    case BLACKBELLY_SKATEPARK:
      return "\x42\xE3\x83\x90\xE3\x82\xB9\xE3\x83\x91\xE3\x83\xBC\xE3\x82\xAF";
    case PORT_MACKEREL:
      return "\xE3\x83\x9B\xE3\x83\x83\xE3\x82\xB1\xE5\x9F\xA0\xE9\xA0\xAD";
    case KELP_DOME:
      return "\xE3\x83\xA2\xE3\x82\xBA\xE3\x82\xAF\xE8\xBE\xB2\xE5\x9C\x92";
    case BLUEFIN_DEPOT:
      return "\xE3\x83\x8D\xE3\x82\xAE\xE3\x83\x88\xE3\x83\xAD\xE7\x82\xAD"
          "\xE9\x89\xB1";
    case MORAY_TOWERS:
      return "\xE3\x82\xBF\xE3\x83\x81\xE3\x82\xA6\xE3\x82\xAA\xE3\x83\x91"
          "\xE3\x83\xBC\xE3\x82\xAD\xE3\x83\xB3\xE3\x82\xB0";
    case CAMP_TRIGGERFISH:
      return "\xE3\x83\xA2\xE3\x83\xB3\xE3\x82\xAC\xE3\x83\xA9\xE3\x82\xAD"
          "\xE3\x83\xA3\xE3\x83\xB3\xE3\x83\x97\xE5\xA0\xB4";
    case HIRAME:
      return "\xE3\x83\x92\xE3\x83\xA9\xE3\x83\xA1\xE3\x81\x8C\xE4\xB8\x98"
          "\xE5\x9B\xA3\xE5\x9C\xB0";
    case MASABA:
      return "\xE3\x83\x9E\xE3\x82\xB5\xE3\x83\x90\xE6\xB5\xB7\xE5\xB3\xA1"
          "\xE5\xA4\xA7\xE6\xA9\x8B";
    default:
      abort();
  }
}

