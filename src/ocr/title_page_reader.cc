#include "ocr/title_page_reader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <glog/logging.h>
#include "util/debugger.h"
#include "util/util.h"

namespace {

double WhiteDiffInternal(const cv::Mat& img1, const cv::Mat& img2) {
  int diff = 0;
  cv::Size img1_size = img1.size();
  cv::Size img2_size = img2.size();
  cv::Size smaller_size(std::min(img1_size.width, img2_size.width),
                        std::min(img1_size.height, img2_size.height));
  cv::Rect rect(cv::Point(0, 0), smaller_size);
  cv::Mat gray1 = img1(rect);
  cv::Mat gray2 = img2(rect);
  for (int i = 0; i < gray1.rows; ++i) {
    for (int j = 0; j < gray1.cols; ++j) {
      bool is_img_1_white = (gray1.at<uchar>(i, j) > 220);
      bool is_img_2_white = (gray2.at<uchar>(i, j) > 220);
      if (is_img_1_white != is_img_2_white) {
        diff++;
      }
    }
  }
  return (double)diff / (smaller_size.width * smaller_size.height);
}

double WhiteDiff(const cv::Mat& img1, const cv::Mat& img2) {
  cv::Size size1 = img1.size();
  cv::Size size2 = img2.size();

  std::vector<double> r;

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      r.push_back(WhiteDiffInternal(
          img1(cv::Rect(i, j, size1.width - i, size1.height -j)),
          img2(cv::Rect(0, 0, size2.width, size2.height))));
      r.push_back(WhiteDiffInternal(
          img1(cv::Rect(0, 0, size1.width, size1.height)),
          img2(cv::Rect(i, j, size2.width - i, size2.height - j))));
    }
  }

  double min_r = 1e+100;

  for (int i = 0; i < r.size(); ++i) {
    if (r[i] < min_r) {
      min_r = r[i];
    }
  }
  return min_r;
}

void TrimBlankImage(const cv::Mat& edge, cv::Rect* out) {
  int margin_l = 0;
  int margin_r = 0;
  int margin_t = 0;
  int margin_b = 0;

  cv::Size size = edge.size();

  bool prev_black = true;
  for (; margin_l < size.width; ++margin_l) {
    bool all_black = true;
    for (int i = 0; i < size.height; ++i) {
      all_black &= (edge.at<uchar>(i, margin_l) == 0);
    }
    if (all_black) {
      prev_black = true;
      continue; // trim this line.
    } else if (prev_black) {
      prev_black = false;
      continue;
    } else {
      margin_l--;
      break;
    }
  }

  for (; margin_r < size.width; ++margin_r) {
    bool all_black = true;
    for (int i = 0; i < size.height; ++i) {
      all_black &= (edge.at<uchar>(i, size.width - margin_r -1) == 0);
    }
    if (all_black) {
      prev_black = true;
      continue; // trim this line.
    } else if (prev_black) {
      prev_black = false;
      continue;
    } else {
      margin_r--;
      break;
    }
  }

  for (; margin_t < size.height; ++margin_t) {
    for (int i = 0; i < size.width; ++i) {
      if (edge.at<uchar>(margin_t, i) != 0)
        goto end_t;
    }
  }
end_t:
  for (; margin_b < size.height; ++margin_b) {
    for (int i = 0; i < size.width; ++i) {
      if (edge.at<uchar>(size.height - margin_b -1, i) != 0)
        goto end_b;
    }
  }
end_b:
  out->x = margin_l;
  out->y = margin_t;
  out->width = size.width - (margin_l + margin_r);
  out->height = size.height - (margin_b + margin_t);
}
}  // namespace

TitlePageReader::TitlePageReader()
    : rule_rect_(730, 380, 460, 85),
      map_rect_(1270, 870, 570, 97),
      area_img_(cv::imread("res/area.png", 0)),
      yagura_img_(cv::imread("res/yagura.png", 0)),
      map_(URCHIN_UNDERPASS),
      rule_(NAWABARI) {
#define LOAD_IMAGE(key, fname) \
  maps_[key] = cv::imread("res/map/" fname ".png", 0)
  LOAD_IMAGE(URCHIN_UNDERPASS, "urchin_underpass");
  LOAD_IMAGE(WALLEYE_WAREHOUSE, "walleye_warehouse");
  LOAD_IMAGE(SALTSPRAY_RIG, "saltspray_rig");
  LOAD_IMAGE(AROWANA_MALL, "arowana_mall");
  LOAD_IMAGE(BLACKBELLY_SKATEPARK, "blackbelly_skatepark");
  LOAD_IMAGE(PORT_MACKEREL, "port_mackerel");
  LOAD_IMAGE(KELP_DOME, "kelp_dome");
  LOAD_IMAGE(BLUEFIN_DEPOT, "bluefin_depot");
  LOAD_IMAGE(MORAY_TOWERS, "moray_towers");
  LOAD_IMAGE(CAMP_TRIGGERFISH, "camp_triggerfish");
  LOAD_IMAGE(HIRAME, "hirame");
#undef LOAD_IMAGE
}

TitlePageReader::~TitlePageReader() {
}

void TitlePageReader::LoadImageSequence(const std::vector<cv::Mat>& images) {
  for (int i = 0; i < images.size(); ++i) {
    cv::Mat tmp;
    cv::Mat tmp2;
    cv::cvtColor(images[i], tmp, CV_RGB2GRAY);
    cv::threshold(tmp, tmp2, 0xE0, 0xFF, CV_THRESH_BINARY);
    ShowAndWaitKey(tmp2);
    if (i == 0)
      image_ = tmp2;
    else
      image_ = image_.mul(tmp2);
  }
  cv::Mat map_image = image_(map_rect_);
  cv::Rect trimmed_map_rect;
  TrimBlankImage(map_image, &trimmed_map_rect);
  map_image = map_image(trimmed_map_rect);
  map_ = DetectMap(map_image, trimmed_map_rect);

  cv::Mat rule_image = image_(rule_rect_);
  cv::Rect trimmed_rule_rect;
  ShowAndWaitKey(rule_image);
  TrimBlankImage(rule_image, &trimmed_rule_rect);
  rule_image = rule_image(trimmed_rule_rect);
  rule_ = DetectRule(rule_image, trimmed_rule_rect);
}

TitlePageReader::Rule TitlePageReader::ReadRule() const {
  return rule_;
}

TitlePageReader::Map TitlePageReader::ReadMap() const {
  return map_;
}

TitlePageReader::Map TitlePageReader::DetectMap(
    const cv::Mat& image, const cv::Rect& rect) {
  double similarity[NUM_OF_MAPS];

  for (int i = 0; i < NUM_OF_MAPS; ++i) {
    cv::Size size = maps_[i].size();
    if (abs(size.width - rect.width) > 20) {
      similarity[i] = 1e+100;
      continue;
    }

    similarity[i] = WhiteDiff(maps_[i], image);
  }

  int min_index = -1;
  double min_value = 1e+50;
  for (int i = 0; i < NUM_OF_MAPS; ++i) {
    if (similarity[i] < min_value) {
      min_index = i;
      min_value = similarity[i];
    }
  }

  if (min_index == -1) {
    ShowAndWaitKey(image);
    LOG(FATAL) << "Unable to detect the map name";
    abort();
  }

  return (Map)min_index;
}

TitlePageReader::Rule TitlePageReader::DetectRule(
    const cv::Mat& image, const cv::Rect& rect) {
  if (rect.width > 400) {
    return NAWABARI;
  }

  double yagura = WhiteDiff(yagura_img_, image);
  double area = WhiteDiff(area_img_, image);

  return (yagura < area) ? YAGURA : AREA;
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
    default:
      abort();
  }
}

