#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/util.h"
#include "util/debugger.h"
#include "ocr/image_clipper.h"
#include <glog/logging.h>

namespace {

int MaskAndWhiteness(const cv::Mat& image, const cv::Mat& mask) {
  cv::Size mask_size = mask.size();
  int rank_sum = 0;
  int black_count;
  for (int i = 0; i < mask_size.height; ++i) {
    const uchar* line = image.ptr<uchar>(i);
    const uchar* mask_line = mask.ptr<uchar>(i);
    for (int j = 0; j < mask_size.width; ++j) {
      if (mask_line[j] == 0x00) {
        rank_sum += (0xFF - line[j]);
      }
      if (line[j] < 0x10)
        black_count++;
    }
  }
  if (abs(black_count - mask_size.width * mask_size.height) < 5)
    return INT_MAX;
  return rank_sum;
}

void TrimBlankImage(const cv::Mat& image, cv::Rect* out) {
  int margin_l = 0;
  int margin_r = 0;
  int margin_t = 0;
  int margin_b = 0;

  cv::Mat edge;
  ExtractWhite(image(*out), &edge);
  cv::Size size = edge.size();

  for (; margin_l < size.width; ++margin_l) {
    for (int i = 0; i < size.height; ++i) {
      if (edge.at<uchar>(i, margin_l) != 0)
        goto end_l;
    }
  }
end_l:
  for (; margin_r < size.width; ++margin_r) {
    for (int i = 0; i < size.height; ++i) {
      if (edge.at<uchar>(i, size.width - margin_r -1) != 0)
        goto end_r;
    }
  }
end_r:
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
  if (margin_l == size.width || margin_r == size.width ||
      margin_t == size.height || margin_b == size.height) {
    // Fuckin' empty name. Do nothing here for avoiding empty rectangle.
    return;
  }

  const int kPadding = 2;
  out->x += std::max(margin_l - kPadding, 0);
  out->y += std::max(margin_t - kPadding, 0);
  out->width -= std::max(margin_l + margin_r - kPadding * 2, 0);
  out->height -= std::max(margin_b + margin_t - kPadding * 2, 0);
}

int getStatus(const cv::Rect& first_rank_rect, int frame_interval,
              int player_indent, int lose_indent, const cv::Mat& rank_img,
              const cv::Mat& bw_image, ImageClipper::PlayerStatus* out) {
  cv::Rect rect = first_rank_rect;

  int you_idx = -1;
  for (int i = 0; i < 8; ++i) {
    if (i == 4)
      rect.y += lose_indent;

    cv::Mat normal_img = bw_image(rect);
    double normal_diff = ImageDiff(rank_img, normal_img);

    cv::Rect player_rect = rect;
    player_rect.x -= player_indent;

    double player_diff = ImageDiff(rank_img, bw_image(player_rect));
    if (normal_diff > player_diff) {
      if (you_idx != -1) {
        LOG(ERROR) << "Found two or more player position.";
        ShowAndWaitKey(bw_image);
        abort();
      }
      out[i] = ImageClipper::YOU;
      you_idx = i;
    } else {
      if (IsBlackImage(normal_img)) {
        out[i] = ImageClipper::VACANCY;
      } else {
        out[i] = ImageClipper::NORMAL;
      }
    }

    rect.y += frame_interval;
  }
  return you_idx;
}

}  // namespace

ImageClipper::ImageClipper(const std::string& fname) {
  cv::Mat image = cv::imread(fname);
  calcRect(image);
  clipImage(image);
}

ImageClipper::ImageClipper(const cv::Mat& image) {
  calcRect(image);
  clipImage(image);
}

ImageClipper::~ImageClipper() {
}

void ImageClipper::calcRect(const cv::Mat& image) {
  const cv::Size size = image.size();
  if (size.width == 1280 && size.height == 720) {
    calcRect720(image);
  } else if (size.width == 1920 && size.height == 1080) {
    calcRect1080(image);
  } else {
    LOG(ERROR) << "Unsupported image size: "
        << size.width << " x " << size.height;
    abort();
  }
}

void ImageClipper::calcRect720(const cv::Mat& image) {
  const int kPlayerDiff = 41;
  const int kFrameInterval = 65;
  const int kLoseIndent = 70;
  const int kRankWinStart = 103;
  const int kPaintPointWidth = 27;
  const int kKillPointWidth = 15;

  static const cv::Rect kFirstRankRect(656, 103, 37, 16);

  cv::Mat bw_image;
  ExtractWhite(image, &bw_image);
  // Check P position. If it is blank, the image is nawabari.
  is_nawabari_ = !IsBlackImage(bw_image(cv::Rect(1109, 117, 18, 22)));

  int player_idx =  getStatus(kFirstRankRect, kFrameInterval, kPlayerDiff,
                              kLoseIndent, cv::imread("res720/rank.png", 0),
                              bw_image, status_);
  for (int i = 0; i < 8; ++i) {
    rects_[i].frame = cv::Rect(632, 94 + kFrameInterval * i, 611, 52);
    rects_[i].name = cv::Rect(808, 99 + kFrameInterval * i, 180, 40);
    rects_[i].weapon = cv::Rect(758, 97 + kFrameInterval * i, 50, 46);
    for (int j = 0; j < 4; ++j) {
      rects_[i].point[j] = cv::Rect(
          1003 + kPaintPointWidth * j, 105 + kFrameInterval * i,
          kPaintPointWidth, 30);
    }
    for (int j = 0; j < 2; ++j) {
      rects_[i].kill[j] = cv::Rect(
          1185 + kKillPointWidth * j, 100 + kFrameInterval * i,
          kKillPointWidth, 17);
      rects_[i].death[j] = cv::Rect(
          1185 + kKillPointWidth * j, 121 + kFrameInterval * i,
          kKillPointWidth, 17);
    }
    if (i < 4) {
      continue;
    }
    rects_[i].frame.y += kLoseIndent;
    rects_[i].name.y += kLoseIndent;
    rects_[i].weapon.y += kLoseIndent;
    for (int j = 0; j < 4; ++j) {
      rects_[i].point[j].y += kLoseIndent;
    }
    for (int j = 0; j < 2; ++j) {
      rects_[i].kill[j].y  += kLoseIndent;
      rects_[i].death[j].y  += kLoseIndent;
    }
  }
  rects_[player_idx].frame.x -= kPlayerDiff;
  rects_[player_idx].frame.width += kPlayerDiff;
  rects_[player_idx].name.x -= kPlayerDiff;
  rects_[player_idx].weapon.x -= kPlayerDiff;

  for (int i = 0; i < 8; ++i) {
    TrimBlankImage(image, &rects_[i].name);
  }
}

void ImageClipper::calcRect1080(const cv::Mat& image) {
  static const int kRankY1080[8] = {158, 255, 353, 450, 653, 750, 848, 945};
  /*
  cv::Rect rect = cv::Rect(1109, 117, 18, 22);
  while (true) {

    cv::Mat img = image_.clone();
    cv::rectangle(img, rect.tl(), rect.br(),
                  cv::Scalar(0, 255, 255), 2, 8);
    cv::imshow("debug", img);
    int c;
    switch(c = cv::waitKey(0)) {
      case 0x10ff52: rect.y--; break;
      case 0x10ff54: rect.y++; break;
      case 0x10ff51: rect.x--; break;
      case 0x10ff53: rect.x++; break;
      case 0x100061: rect.width--; break;
      case 0x100073: rect.width++; break;
      case 0x100064: rect.height--; break;
      case 0x100066: rect.height++; break;
      case 0x10000a:
        LOG(ERROR) << rect.x << ", " << rect.y << ", " << rect.width << ", " << rect.height;
        goto next;
      default:
        LOG(ERROR) << std::hex << c;
    }
  }
next:
  cv::Mat write_img = image_;
  ExtractWhite(image_, &write_img);
  cv::imwrite("debug.png", write_img(rect));
  */
}

cv::Mat ImageClipper::DrawDebugInfo(const cv::Mat& image) const {
  cv::Mat result = image.clone();
  for (int i = 0; i < 8; ++i) {
    if (status_[i] == VACANCY)
      continue;
    const ClippingRect& rect = rects_[i];

    cv::rectangle(result, rect.frame.tl(), rect.frame.br(),
                  cv::Scalar(0, 255, 255), 2, 8);
    cv::rectangle(result, rect.name.tl(), rect.name.br(),
                  cv::Scalar(255, 0, 0), 2, 8);
    cv::rectangle(result, rect.weapon.tl(), rect.weapon.br(),
                  cv::Scalar(0, 0, 255), 2, 8);
    cv::rectangle(result, rect.kill[0].tl(), rect.kill[0].br(),
                  cv::Scalar(0, 255, 0), 2, 8);
    cv::rectangle(result, rect.kill[1].tl(), rect.kill[1].br(),
                  cv::Scalar(0, 255, 0), 2, 8);
    cv::rectangle(result, rect.death[0].tl(), rect.death[0].br(),
                  cv::Scalar(0, 0, 255), 2, 8);
    cv::rectangle(result, rect.death[1].tl(), rect.death[1].br(),
                  cv::Scalar(0, 0, 255), 2, 8);
    if (is_nawabari_) {
      cv::rectangle(result, rect.point[0].tl(), rect.point[0].br(),
                    cv::Scalar(255, 255, 0), 2, 8);
      cv::rectangle(result, rect.point[1].tl(), rect.point[1].br(),
                    cv::Scalar(255, 255, 0), 2, 8);
      cv::rectangle(result, rect.point[2].tl(), rect.point[2].br(),
                    cv::Scalar(255, 255, 0), 2, 8);
      cv::rectangle(result, rect.point[3].tl(), rect.point[3].br(),
                    cv::Scalar(255, 255, 0), 2, 8);
    }
  }
  return result;
}

void ImageClipper::ShowDebugImage(const cv::Mat& image) const {
  ShowAndWaitKey(DrawDebugInfo(image));
}

void ImageClipper::clipImage(const cv::Mat& image) {
  for (int i = 0; i < 8; ++i) {
    if (status_[i] == VACANCY)
      continue;
    ClippedImage& clip = images_[i];
    const ClippingRect& rect = rects_[i];

    clip.result = cv::Mat(image, rect.frame);
    clip.name = cv::Mat(image, rect.name);
    clip.weapon = cv::Mat(image, rect.weapon);
    clip.kill[0] = cv::Mat(image, rect.kill[0]);
    clip.kill[1] = cv::Mat(image, rect.kill[1]);
    clip.death[0] = cv::Mat(image, rect.death[0]);
    clip.death[1] = cv::Mat(image, rect.death[1]);
    if (is_nawabari_) {
      clip.point[0] = cv::Mat(image, rect.point[0]);
      clip.point[1] = cv::Mat(image, rect.point[1]);
      clip.point[2] = cv::Mat(image, rect.point[2]);
      clip.point[3] = cv::Mat(image, rect.point[3]);
    }
  }
}

