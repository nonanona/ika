#pragma once

#include "ocr/image_clipper.h"

class TitlePageReader {
 public:
  TitlePageReader();
  ~TitlePageReader();

  enum Rule {
    UNKNOWN_RULE = 0,
    NAWABARI,  // 487,254 - 303x52
    AREA,  // 534,251 - 206x55
    YAGURA,  // 535,251 - 207x55
    HOKO,  // 488,251 - 300x55

    NUM_OF_RULES,
  };

  enum Map {
    UNKNOWN_MAP = 0,
    URCHIN_UNDERPASS,  // DEKARAIN KOUKASHITA
    WALLEYE_WAREHOUSE,  // HAKOFUGU DEPOT
    SALTSPRAY_RIG,  // SIONOME YUDEN
    AROWANA_MALL,  // AROWANA MALL
    BLACKBELLY_SKATEPARK,  // B BASS PARK
    PORT_MACKEREL,  // HOKKE PORT
    KELP_DOME,  // MOZUKU NOEN
    BLUEFIN_DEPOT,  // NEGITORO TANKO
    MORAY_TOWERS,   // TACHIUO PARKING
    CAMP_TRIGGERFISH,  // MONGARA CAMP
    HIRAME,
    MASABA,

    NUM_OF_MAPS,
  };

  void initialize(const cv::Size& size);
  void LoadImage(const cv::Mat& image);

  Rule ReadRule() const;
  Map ReadMap() const;

  static const char* GetRuleString(Rule rule);
  static const char* GetMapString(Map map);

  void ShowDebugImage(bool with_rect) const;

 private:
  void initialize720();
  void initialize1080();

  cv::Mat maps_[NUM_OF_MAPS];
  cv::Rect map_rects_[NUM_OF_MAPS];

  cv::Mat rules_[NUM_OF_RULES];
  cv::Rect rule_rects_[NUM_OF_RULES];

  Map map_;
  Rule rule_;
  bool is_initialized_;
};
