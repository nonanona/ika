#include "learning_command.h"

#include <glog/logging.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <set>

#include "util/util.h"
#include "util/ioutil.h"
#include "util/debugger.h"
#include "ocr/result_page_reader.h"
#include "ocr/title_page_reader.h"
#include "ocr/name_tracker.h"
#include "ocr/killdeath_classifier.h"
#include "ocr/paintpoint_classifier.h"
#include "scene_analyzer/game_scene_extractor.h"
#include "printer.h"

LearningCommand::LearningCommand() : xv_ratio_(0.2), train_count_(-1) {
}

LearningCommand::~LearningCommand() {
}

bool LearningCommand::ProcessArgs(int argc, char** argv) {
  image_dir_ = GetCmdOption(argv + 1, argv + argc, "-i");
  out_file_ = GetCmdOption(argv + 1, argv + argc, "-o");
  is_debug_ = HasCmdOption(argv + 1, argv + argc, "--debug");
  std::string mode_str = GetCmdOption(argv + 1, argv + argc, "--mode");
  if (mode_str == "killdeath") {
    mode_ = KILL_DEATH;
  } else if (mode_str == "paintpoint") {
    mode_ = PAINT_POINT;
  } else if (mode_str == "weapon") {
    mode_ = WEAPON;
  } else {
    LOG(FATAL) << "mode str must be killdeath, paintpoint or weapon";
  }

  for (int i = 0; i < 10; ++i) {
    std::string path = image_dir_ + "/" + char(i + '0');
    if (!IsDirectory(path)) {
      LOG(FATAL) << path << " must exists";
    }
  }

  std::string xv_ratio = GetCmdOption(argv + 1, argv + argc, "--xv-ratio");
  std::string train_count =
      GetCmdOption(argv + 1, argv + argc, "--train-count");
  if (!train_count.empty() && !xv_ratio.empty()) {
    LOG(FATAL) << "Do not specify --train-count --xv-ratio at the same time";
    abort();
  }
  if (xv_ratio.empty()) {
    xv_ratio_ = 0.2;
  } else {
    xv_ratio_ = atof(xv_ratio.c_str());
  }

  if (train_count.empty()) {
    train_count_ = -1;
  } else {
    train_count_ = atoi(train_count.c_str());
  }

  return !image_dir_.empty();
}

void LearningCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [image dir] --mode [killdeath|paintpoint|weapon]\
         [--xv-ratio] [xv ratio] [--debug]\n",
         myself, GetCommandName());
}

void LearningCommand::Run() {
  int kMaxIter = 1000;
  double criteria = 0.5;
  for (int i = 0 ; i < kMaxIter; ++i) {
    double res = DoLearning(criteria);
    if (fabs(res - 1.0) < 1e-8) {
      return;
    }
    if (res > criteria)
      criteria = res;
  }
}

double LearningCommand::DoLearning(double criteria) {
  srand(time(NULL));
  std::vector<std::string> train_files[10];
  std::vector<std::string> verify_files[10];

  if (train_count_ == -1) {
    LOG(INFO) << "Using " << xv_ratio_ 
        << " of train data will be used for Cross Varidation";
  } else {
    LOG(INFO) << "Using " << train_count_
        << " of train data will be used for Training";
  }

  int train_image_count = 0;
  int verify_image_count = 0;
  for (int i = 0 ; i < 10; ++i) {
    std::string path = image_dir_ + "/" + char(i + '0');
    std::vector<std::string> files = ScanDir(path + "/");

    if (train_count_ == -1 ) {
      for (int j = 0; j < files.size(); ++j) {
        double r = (double)rand() / (RAND_MAX + 1.0);
        if (r < xv_ratio_) {
          verify_files[i].push_back(files[j]);
          verify_image_count++;
        } else {
          train_files[i].push_back(files[j]);
          train_image_count++;
        }
      }
    } else {
      if (train_count_ >= files.size()) {
        LOG(FATAL) << "Requested larger count for training data.";
      }
      std::set<size_t> idx;
      for (;;) {
        double r = (double)rand() / (RAND_MAX + 1.0);
        size_t cand = (size_t)(r * files.size());
        if (idx.find(cand) != idx.end())
          continue;
        idx.insert(cand);
        if (idx.size() == train_count_)
          break;
      }

      for (int j = 0; j < files.size(); ++j) {
        if (idx.find(j) == idx.end()) {
          verify_files[i].push_back(files[j]);
          verify_image_count++;
        } else {
          train_files[i].push_back(files[j]);
          train_image_count++;
        }
      }
    }
  }

  LOG(INFO) << "Total Data Count: " << (train_image_count + verify_image_count);
  for (int i = 0; i < 10; ++i) {
    LOG(INFO) << "Data for " << i << " (Train/Verify): "
        << train_files[i].size() << " / " << verify_files[i].size();
  }

  cv::Mat sample;
  if (mode_ == KILL_DEATH)
    KillDeathClassifier::Prepare(cv::imread(train_files[0][0]), &sample);
  else if (mode_ == PAINT_POINT)
    PaintPointClassifier::Prepare(cv::imread(train_files[0][0]), &sample);
  else
    abort();

  cv::Size data_size = sample.size();
  cv::Mat train_data(train_image_count, data_size.width * data_size.height,
                     CV_32FC1);
  cv::Mat train_label(train_image_count, 1, CV_32SC1);

  int train_line = 0;
  for (int i = 0; i < 10; ++i) {
    for (size_t j = 0; j < train_files[i].size(); ++j) {
      cv::Mat tmp;
      if (mode_ == KILL_DEATH)
        KillDeathClassifier::Prepare(cv::imread(train_files[i][j]), &tmp);
      else if (mode_ == PAINT_POINT)
        PaintPointClassifier::Prepare(cv::imread(train_files[i][j]), &tmp);
      else
        abort();

      for (int ii = 0; ii < tmp.cols; ++ii) {
        train_data.at<float>(train_line, ii) = tmp.at<float>(0, ii);
      }
      train_label.at<int32_t>(train_line, 0) = i;
      train_line++;
    }
  }

  LOG(INFO) << "Start Training";
  cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
  svm->setType(cv::ml::SVM::C_SVC);
  svm->setKernel(cv::ml::SVM::RBF);
  svm->setGamma(3);
  cv::Ptr<cv::ml::TrainData> train_data2 =
      cv::ml::TrainData::create(train_data, cv::ml::ROW_SAMPLE, train_label);
  svm->trainAuto(train_data2);
  LOG(INFO) << "Training Done";

  int total_correct = 0;
  for (int i = 0; i < 10; ++i) {
    int correct_count = 0;
    for (size_t j = 0; j < verify_files[i].size(); ++j) {
      cv::Mat tmp;
      if (mode_ == KILL_DEATH)
        KillDeathClassifier::Prepare(cv::imread(verify_files[i][j]), &tmp);
      else if (mode_ == PAINT_POINT)
        PaintPointClassifier::Prepare(cv::imread(verify_files[i][j]), &tmp);
      else
        abort();
      if (svm->predict(tmp) == i) {
        correct_count++;
        total_correct++;
      }
    }
    LOG(INFO) << "Precision: " << i << ": "
        << correct_count << "/" << verify_files[i].size()
        << " (" << (double)(100.0 * correct_count) / verify_files[i].size()
        << "%)";
  }
  double prec = (double)(total_correct)/(double)(verify_image_count);
  LOG(INFO) << "Total Accuracy is " << 100.0 * prec << "%";
  if (prec > criteria && !out_file_.empty()) {
    LOG(INFO) << "Saving learned params to " << out_file_;
    svm->save(out_file_);
  }
  return prec;
}
