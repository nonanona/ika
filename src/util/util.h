#pragma once

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

void PrintRect(const cv::Rect& rect);
void PrintSize(const cv::Size& size);
void PrintPoint(const cv::Point& pos);
std::vector<std::string> ScanDir(const std::string& dir);
std::string GetCmdOption(char** begin, char** end, const std::string& opt);
bool HasCmdOption(char** begin, char** end, const std::string& opt);
void ExtractWhite(const cv::Mat& in, cv::Mat* out);
bool IsBlackImage(const cv::Mat& in);
bool IsWhiteImage(const cv::Mat& in);
double ImageDiff(const cv::Mat& l, const cv::Mat& r);
