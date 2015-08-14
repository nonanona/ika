#pragma once

#include <string>

bool IsDirectory(const std::string& path);
bool IsFile(const std::string& path);
bool FileExists(const std::string& path);
bool MakeDir(const std::string& path);
void MakeSureDirExists(const std::string& path);
