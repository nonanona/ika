#include "component_extract_command.h"

#include <glog/logging.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util/util.h"
#include "util/ioutil.h"
#include "util/debugger.h"
#include "ocr/image_clipper.h"
#include "ocr/classifier.h"
#include "ocr/killdeath_classifier.h"

ComponentExtractCommand::ComponentExtractCommand() {
}

ComponentExtractCommand::~ComponentExtractCommand() {
}

bool ComponentExtractCommand::ProcessArgs(int argc, char** argv) {
  image_path_ = GetCmdOption(argv + 1, argv + argc, "-i");
  output_dir_ = GetCmdOption(argv + 1, argv + argc, "-o");
  is_nawabari_ = HasCmdOption(argv + 1, argv + argc, "--nawabari");
  is_debug_ = HasCmdOption(argv + 1, argv + argc, "--debug");
  is_overwrite_ = HasCmdOption(argv + 1, argv + argc, "--overwrite");
  return !image_path_.empty() && !output_dir_.empty();
}

void ComponentExtractCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [image path] -o [output] [--debug]\n",
         myself, GetCommandName());
}

void ComponentExtractCommand::Run() {
  MakeSureDirExists(output_dir_);

  if (IsDirectory(image_path_)) {
    LOG(INFO) << "Directory is passed. Processing all file in the directory";
    std::vector<std::string> files = ScanDir(image_path_);
    for (int i = 0; i < files.size(); ++i) {
      LOG(INFO) << "Processing: " << files[i];
      SaveToFile(files[i]);
    }
  } else {
    LOG(INFO) << "Image file is passed.";
    SaveToFile(image_path_);
  }
}

int ComponentExtractCommand::GetNearestFileIndex(int index,
                                                 const std::string& dir,
                                                 std::string* out) {
  if (is_overwrite_) {
    char buf[260];
    snprintf(buf, 260, "%s/%05d.png", dir.c_str(), index + 1);
    *out = buf;
    return index + 1;
  }
  const int MAX_INDEX = 10000000;
  for (; index < MAX_INDEX; index++) {
    char buf[260];
    snprintf(buf, 260, "%s/%05d.png", dir.c_str(), index);
    if (!FileExists(buf)) {
      *out = buf;
      return index;
    }
  }
  *out = "/";
  return -1;
}

int PredictKillDeathImage(const cv::Mat& image) {
  static KillDeathClassifier* cls = NULL;
  if (cls == NULL) {
     cls = new KillDeathClassifier();
  }

  return cls->Predict(image);
}

int PredictPaintPointImage(const cv::Mat& image) {
  static Classifier* cls = NULL;
  if (cls == NULL) {
     cls = new Classifier("res/point_param", 2.0);
  }

  cv::Mat buf;
  cv::Canny(image, buf, 50, 200);
  return cls->Predict(buf, 0.5);
}

void ComponentExtractCommand::SaveToFile(const std::string& path) {
  printf("Processing: %s\n", path.c_str()); fflush(stdout);

  const std::string kNameDir = output_dir_ + "/name";
  MakeSureDirExists(kNameDir);

  const std::string kWeaponDir = output_dir_ + "/weapon";
  MakeSureDirExists(kWeaponDir);

  const std::string kKillDeathDir = output_dir_ + "/killdeath/";
  MakeSureDirExists(kKillDeathDir);

  const std::string kPaintPointDir = output_dir_ + "/paintpoint/";
  MakeSureDirExists(kPaintPointDir);

  for (int i = 0; i < 10; ++i) {
    std::string subdir = kKillDeathDir + (char)('0' + i);
    MakeSureDirExists(subdir);

    subdir = kPaintPointDir + (char)('0' + i);
    MakeSureDirExists(subdir);
  }

  ImageClipper ic(path, is_nawabari_);

  char predict[2] = {};
  cv::Mat tmp;
  for (int i = 0; i < 8; ++i) {
    const ImageClipper::ClippedImage& image = ic.getImage(i);
    std::string path;

    name_index_ = GetNearestFileIndex(name_index_, kNameDir, &path);
    cv::imwrite(path, image.name);

    weapon_index_ = GetNearestFileIndex(weapon_index_, kWeaponDir, &path);
    cv::imwrite(path, image.weapon);

    predict[0] = '0' + PredictKillDeathImage(image.kill[0]);
    kill_death_index_ =
        GetNearestFileIndex(kill_death_index_, kKillDeathDir + predict, &path);
    ExtractWhite(image.kill[0], &tmp);
    if (!IsBlackImage(tmp))
      cv::imwrite(path, image.kill[0]);

    predict[0] = '0' + PredictKillDeathImage(image.kill[1]);
    kill_death_index_ =
        GetNearestFileIndex(kill_death_index_, kKillDeathDir + predict, &path);
    ExtractWhite(image.kill[1], &tmp);
    if (!IsBlackImage(tmp))
      cv::imwrite(path, image.kill[1]);

    predict[0] = '0' + PredictKillDeathImage(image.death[0]);
    kill_death_index_ =
        GetNearestFileIndex(kill_death_index_, kKillDeathDir + predict, &path);
    ExtractWhite(image.death[0], &tmp);
    if (!IsBlackImage(tmp))
      cv::imwrite(path, image.death[0]);

    predict[0] = '0' + PredictKillDeathImage(image.death[1]);
    kill_death_index_ =
        GetNearestFileIndex(kill_death_index_, kKillDeathDir + predict, &path);
    ExtractWhite(image.death[1], &tmp);
    if (!IsBlackImage(tmp))
      cv::imwrite(path, image.death[1]);

    if (is_nawabari_) {
      for (int i = 0; i < 4; ++i) {
        predict[0] = '0' + PredictPaintPointImage(image.point[i]);
        paintpoint_index_ =
            GetNearestFileIndex(paintpoint_index_,
                                kPaintPointDir + predict, &path);
        ExtractWhite(image.point[i], &tmp);
        if (!IsBlackImage(tmp))
          cv::imwrite(path, tmp);
      }
    }
  }
}
