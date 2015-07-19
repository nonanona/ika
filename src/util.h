#pragma once

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

void PrintRect(const cv::Rect& rect);
void PrintSize(const cv::Size& size);
void PrintPoint(const cv::Point& pos);
std::vector<std::string> ScanDir(const std::string& dir);
