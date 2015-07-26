#pragma once

#include "ocr/image_clipper.h"

class TitlePageReader {
 public:
  TitlePageReader();
  ~TitlePageReader();

  enum Rule {
    NAWABARI,
    AREA,
    YAGURA,
  };

  enum Map {
    URCHIN_UNDERPASS = 0,  // DEKARAIN KOUKASHITA
    WALLEYE_WAREHOUSE,  // HAKOFUGU DEPOT
    SALTSPRAY_RIG,  // SIONOME YUDEN
    AROWANA_MALL,  // AROWANA MALL
    BLACKBELLY_SKATEPARK,  // B BASS PARK
    PORT_MACKEREL,  // HOKKE PORT
    KELP_DOME,  // MOZUKU NOEN
    BLUEFIN_DEPOT,  // NEGITORO TANKO
    MORAY_TOWERS,   // TACHIUO PARKING
    CAMP_TRIGGERFISH,  // MONGARA CAMP

    NUM_OF_MAPS,
  };

  void LoadImageSequence(const std::vector<cv::Mat>& images);

  Rule ReadRule() const;
  Map ReadMap() const;

  static const char* GetRuleString(Rule rule);
  static const char* GetMapString(Map map);

  void ShowDebugImage(bool with_rect) const;

 private:
  Map DetectMap(const cv::Mat& image, const cv::Rect& rect);
  Rule DetectRule(const cv::Mat& image, const cv::Rect& rect);

  cv::Mat image_;
  const cv::Rect rule_rect_;
  const cv::Rect map_rect_;
  const cv::Mat area_img_;
  const cv::Mat yagura_img_;

  cv::Mat maps_[NUM_OF_MAPS];
  Map map_;
  Rule rule_;
};
