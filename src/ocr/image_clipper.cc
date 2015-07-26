#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/debugger.h"
#include "ocr/classifier.h"
#include "ocr/image_clipper.h"
#include <glog/logging.h>

namespace {

int kRankY[] = { 158, 255, 353, 450, 653, 750, 848, 945 };

int MaskAndWhiteness(const cv::Mat& image, const cv::Mat& mask) {
  cv::Size mask_size = mask.size();
  int rank_sum = 0;
  for (int i = 0; i < mask_size.height; ++i) {
    const uchar* line = image.ptr<uchar>(i);
    const uchar* mask_line = mask.ptr<uchar>(i);
    for (int j = 0; j < mask_size.width; ++j) {
      if (mask_line[j] == 0x00) {
        rank_sum += (0xFF - line[j]);
      }
    }
  }
  return rank_sum;
}

int FindMyIndex(const cv::Mat& image) {
  cv::Mat mask = cv::imread("res/rank.bmp", CV_LOAD_IMAGE_GRAYSCALE);
  cv::Size mask_size = mask.size();

  int max_i = -1;
  int max_value = -1;
  for (int i = 0; i < sizeof(kRankY)/sizeof(kRankY[0]); ++i) {
    cv::Rect target_rect(985, kRankY[i], mask_size.width, mask_size.height);
    cv::Mat candidate(image, target_rect);
    int whiteness = MaskAndWhiteness(candidate, mask);

    if (whiteness > max_value) {
      max_i = i;
      max_value = whiteness;
    }
  }
  return max_i;
}

void TrimBlankImage(const cv::Mat& image, cv::Rect* out) {
  int margin_l = 0;
  int margin_r = 0;
  int margin_t = 0;
  int margin_b = 0;

  cv::Mat edge;
  cv::Canny(image(*out), edge, 50, 200);
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
  out->x += margin_l;
  out->y += margin_t;
  out->width -= (margin_l + margin_r);
  out->height -= (margin_b + margin_t);
}

}  // namespace

ImageClipper::ImageClipper(const std::string& fname, bool is_nawabari)
    : is_nawabari_(is_nawabari),
      my_index_(-1),
      image_(cv::imread(fname)),
      gray_image_(cv::imread(fname, CV_LOAD_IMAGE_GRAYSCALE)) {
  calculateRect();
  clippingImage();
}

ImageClipper::ImageClipper(const cv::Mat& image, bool is_nawabari)
    : is_nawabari_(is_nawabari),
      my_index_(-1),
      image_(image) {
  cv::cvtColor(image, gray_image_, CV_RGB2GRAY);
  calculateRect();
  clippingImage();
}

ImageClipper::~ImageClipper() {
}

void ImageClipper::calculateRectInternal(int index, bool is_player) {
  ClippingRect& rect = rects_[index];

  rect.result = cv::Rect(cv::Point(870, kRankY[index] - 20),
                         cv::Point(1890, kRankY[index] + 65));
  const int margin = is_player? 62 : 0;
  rect.name = cv::Rect(cv::Point(1215 - margin, kRankY[index] + 10),
                       cv::Point(1475 - margin, kRankY[index] + 40));
  TrimBlankImage(image_, &rect.name);

  const cv::Size kd_size(21, 26);
  rect.kill[0] = cv::Rect(cv::Point(1780, kRankY[index] - 6), kd_size);
  rect.kill[1] = cv::Rect(cv::Point(1801, kRankY[index] - 6), kd_size);
  rect.death[0] = cv::Rect(cv::Point(1780, kRankY[index] + 26), kd_size);
  rect.death[1] = cv::Rect(cv::Point(1801, kRankY[index] + 26), kd_size);

  if (is_nawabari_) {
    const int p_left = 1509;
    const cv::Size p_size(39, 46);
    rect.point[0] = cv::Rect(cv::Point(p_left, kRankY[index] + 1), p_size);
    rect.point[1] = cv::Rect(cv::Point(p_left + p_size.width,
                                       kRankY[index] + 1), p_size);
    rect.point[2] = cv::Rect(cv::Point(p_left + p_size.width * 2 ,
                                       kRankY[index] + 1), p_size);
    rect.point[3] = cv::Rect(cv::Point(p_left + p_size.width * 3 ,
                                       kRankY[index] + 1), p_size);
  }
}

void ImageClipper::calculateRect() {
  my_index_ = FindMyIndex(gray_image_);
  for (int i = 0; i < 8; ++i) {
    calculateRectInternal(i, i == my_index_);
  }
}

void ImageClipper::ShowDebugImage(bool with_rect) const {
  cv::Mat result = image_.clone();
  if (with_rect) {
    for (int i = 0; i < 8; ++i) {
      const ClippingRect& rect = rects_[i];

      cv::rectangle(result, rect.result.tl(), rect.result.br(),
                    cv::Scalar(0, 255, 255), 2, 8);
      cv::rectangle(result, rect.name.tl(), rect.name.br(),
                    cv::Scalar(255, 0, 0), 2, 8);
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
  }
  ShowAndWaitKey(result);
}

void ImageClipper::clippingImage() {
  for (int i = 0; i < 8; ++i) {
    clippingImageInternal(i);
  }
}

void ImageClipper::clippingImageInternal(int index) {
  const ClippingRect& rect = getRect(index);
  ClippedImage& image = images_[index];

  image.result = cv::Mat(image_, rect.result);
  image.name = cv::Mat(image_, rect.name);
  image.kill[0] = cv::Mat(image_, rect.kill[0]);
  image.kill[1] = cv::Mat(image_, rect.kill[1]);
  image.death[0] = cv::Mat(image_, rect.death[0]);
  image.death[1] = cv::Mat(image_, rect.death[1]);
  if (is_nawabari_) {
    image.point[0] = cv::Mat(image_, rect.point[0]);
    image.point[1] = cv::Mat(image_, rect.point[1]);
    image.point[2] = cv::Mat(image_, rect.point[2]);
    image.point[3] = cv::Mat(image_, rect.point[3]);
  }
}
