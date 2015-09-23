#include "camera_capture_command.h"

#include <glog/logging.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <list>

#include "util/util.h"
#include "util/debugger.h"
#include "ocr/result_page_reader.h"
#include "ocr/title_page_reader.h"
#include "ocr/name_tracker.h"
#include "scene_analyzer/game_scene_extractor.h"
#include "template/output_handler.h"
#include "template/stdout_output_handler.h"

namespace {

const int kBillion = 1000000000LL;

class TimeInterval {
 public:
  int64_t diff() const;
  static std::string PrettyString(int64_t num);

  void start() {
    clock_gettime(CLOCK_REALTIME, &begin_);
  }

  int64_t snapshot() {
    clock_gettime(CLOCK_REALTIME, &end_);
    return diff();
  }

  void stop() {
    clock_gettime(CLOCK_REALTIME, &end_);
  }

 private:
  timespec begin_;
  timespec end_;
};

class Timer {
 public:
  explicit Timer(TimeInterval* interval) : interval_(interval) {
    interval_->start();
  }

  ~Timer() {
    interval_->snapshot();
  }

 private:
  TimeInterval* interval_;
};

int64_t TimeInterval::diff() const {
  timespec tmp;
  if (end_.tv_nsec < begin_.tv_nsec) {
    tmp.tv_sec = end_.tv_sec - begin_.tv_sec - 1;
    tmp.tv_nsec = kBillion + end_.tv_nsec - begin_.tv_nsec;
  } else {
    tmp.tv_sec = end_.tv_sec - begin_.tv_sec;
    tmp.tv_nsec = end_.tv_nsec - begin_.tv_nsec;
  }
  return (int64_t)tmp.tv_sec * kBillion + tmp.tv_nsec; 
}

std::string TimeInterval::PrettyString(int64_t num) {
  if (num == 0) {
    return "0";
  }

  std::ostringstream os;
  int iter = 0;
  while (num > 0) {
    os << num % 10;
    num /= 10;
    iter++;
    if (iter == 3) {
      os << ",";
      iter = 0;
    }
  }
  std::string reversed = os.str();
  if (reversed[reversed.length() -1] == ',') {
    reversed.erase(reversed.length() - 1);
  }
  return std::string(reversed.rbegin(), reversed.rend());
}

class CenterImageHandler {
 public:
  CenterImageHandler() : oldest_pos_(0), latest_pos_(0) {
  }

  void reset() {
    buffer_.clear();
    oldest_pos_ = 0;
    latest_pos_ = 0;
  }

  void push(const cv::Mat& image) {
    buffer_.push_back(image);
    latest_pos_++;
    for (; oldest_pos_ < (latest_pos_ / 2) + 1; ++oldest_pos_) {
      buffer_.pop_front();
    }
  }

  const cv::Mat& get() {
    return buffer_.front();
  }

  bool hasBuffer() const {
    return !buffer_.empty();
  }

 private:
  std::list<cv::Mat> buffer_;
  size_t oldest_pos_;
  size_t latest_pos_;
};

class IkaStateTracker {
 public:
  enum State {
    UNKNOWN = 0,
    LOADING,
    TITLE,
    BEGIN_OF_BATTLE,
    IN_BATTLE,
    END_OF_BATTLE,
    RESULT,

    NUM_OF_STATE
  };

  const char* getStateMessage() {
    switch (state_) {
      case UNKNOWN: return "Unknown";
      case LOADING: return "Loading";
      case TITLE: return "Title";
      case BEGIN_OF_BATTLE: return "Begin of Battle";
      case IN_BATTLE: return "In Battle";
      case END_OF_BATTLE: return "End of Battle";
      case RESULT: return "Result";
      default: return "OUT-OF-RANGE";
    }
  };

  struct BattleInfo {
    cv::Mat title;
    cv::Mat result;
    time_t begin_time;
    time_t end_time;

    TitlePageReader::Rule rule;
    TitlePageReader::Map map;

    int kill[8];
    int death[8];
    int paint_point[8];
    ImageClipper::PlayerStatus status[8];
    cv::Mat name_image[8];
  };

  size_t getBattleCount() {
    return current_battle_;
  }

  const BattleInfo& getBattleImage(size_t index) {
    return result_[index];
  }

  IkaStateTracker(const cv::Size& size)
      : state_(UNKNOWN), analyzer_(SceneAnalyzer(size, true)),
        current_battle_(0)  {
    result_.push_back(BattleInfo());
    tpr.initialize(size, true);
  }

  State getCurrentState() const {
    return state_;
  }

  bool update(const cv::Mat& frame) {
    State prev_state = state_;

    switch (state_) {
      case UNKNOWN:
        if (analyzer_.IsBlackoutScene(frame)) {
          state_ = LOADING;
          result_[current_battle_].begin_time = time(NULL);
        } else {
          // Do nothing.
        }
        break;
      case LOADING:
        if (!analyzer_.IsBlackoutScene(frame)) {
          state_ = TITLE;
          title_timer_.start();
          title_image_handler_.reset();
        } else {
          // Do nothing.
        }
        break;
      case TITLE:
        if (analyzer_.IsWhiteoutScene(frame)) {
          state_ = BEGIN_OF_BATTLE;
          if (title_image_handler_.hasBuffer()) {
            result_[current_battle_].title = title_image_handler_.get();

            tpr.LoadImage(result_[current_battle_].title, true);
            result_[current_battle_].rule = tpr.ReadRule();
            result_[current_battle_].map = tpr.ReadMap();

            title_image_handler_.reset();
          }
        } else if (analyzer_.IsBlackoutScene(frame)) {
          state_ = LOADING;
        } else {
          const int64_t kTitleTimeout = 60LL * kBillion;  // 60sec
          const int64_t kTitleImageTimeout = 5LL * kBillion; // 5sec;
          const int64_t diff = title_timer_.snapshot();
          if (diff < kTitleImageTimeout) {  // capture only first 5 sec.
            title_image_handler_.push(frame);
          }

          if (diff > kTitleTimeout) {
            state_ = UNKNOWN;
          }
        }
        break;
      case BEGIN_OF_BATTLE:
        if (analyzer_.IsBattleScene(frame)) {
          state_ = IN_BATTLE;
        } else {
          // Do nothing.
        }
        break;
      case IN_BATTLE:
        if (!analyzer_.IsBattleScene(frame) &&
            !analyzer_.IsBlackoutScene(frame)) {
          state_ = END_OF_BATTLE;
        } else {
          // Do nothing.
        }
        break;
      case END_OF_BATTLE:
        if (analyzer_.IsResultScene(frame)) {
          state_ = RESULT;
        } else {
          // Do nothing.
        }
        break;
      case RESULT:
        if (!analyzer_.IsResultScene(frame)) {
          result_[current_battle_].result = result_image_handler_.get();

          rpr.LoadImage(result_[current_battle_].result, true /* camera */);
          for (int i = 0; i < 8; ++i) {
            result_[current_battle_].kill[i] = rpr.ReadKillCount(i);
            result_[current_battle_].death[i] = rpr.ReadDeathCount(i);
            result_[current_battle_].status[i] = rpr.GetPlayerStatus(i);
            result_[current_battle_].name_image[i] = rpr.GetNameImage(i);
            if (rpr.IsNawabari()) {
              result_[current_battle_].paint_point[i] = rpr.ReadPaintPoint(i);
            }
          }

          result_image_handler_.reset();

          result_.push_back(BattleInfo());
          current_battle_++;

          state_ = UNKNOWN;
          result_[current_battle_].end_time = time(NULL);
        } else {
          result_image_handler_.push(frame);
        }
        break;
      default:
        LOG(ERROR) << "Invalid state:" << state_;
        abort();
        break;
    }

    return prev_state != state_;
  }

 private:
  CenterImageHandler title_image_handler_;
  CenterImageHandler result_image_handler_;
  ResultPageReader rpr;
  TitlePageReader tpr;
  State state_;
  TimeInterval title_timer_;
  SceneAnalyzer analyzer_;
  size_t current_battle_;
  std::vector<BattleInfo> result_;
};

}  // namespace

CameraCaptureCommand::CameraCaptureCommand() {
}

CameraCaptureCommand::~CameraCaptureCommand() {
}

bool CameraCaptureCommand::ProcessArgs(int argc, char** argv) {
  return true;
}

void CameraCaptureCommand::PrintUsage(const char* myself) {
  printf("Usage: %s %s -i [image path] [--debug]\n",
         myself, GetCommandName());
}

void CameraCaptureCommand::Run() {
  // TODO: implement enemrate capture devices.
  cv::VideoCapture vc(0);

  if (!vc.isOpened()) {
    LOG(ERROR) << "Unable to open camera device";
    abort();
  }

  cv::namedWindow("preview", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::namedWindow("result", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  cv::namedWindow("title", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);

  cv::Mat raw_frame;

  cv::Size size(vc.get(CV_CAP_PROP_FRAME_WIDTH),
                vc.get(CV_CAP_PROP_FRAME_HEIGHT));

  StdoutOutputHandler handler;
  NameTracker name_tracker;
  handler.Initialize();

  int i  = 0;
  IkaStateTracker tracker(size);
  float a = 10.0;
  uchar lut[256];

  bool capturing = false;
  for (int i = 0; i < 256; i++) 
    lut[i] = 255.0 / (1 + exp( -a * ( i - 128) / 255));  // sigmoid
 
  int in_result = 0;
  while (true) {
    TimeInterval fps_interval;
    {
      Timer timer(&fps_interval);
      int k = cv::waitKey(1);
      vc >> raw_frame;
      if (raw_frame.empty())
        break;

      TimeInterval interval;
      {
        Timer timer(&interval);
        cv::Mat frame;
        LUT(raw_frame, cv::Mat(cv::Size(256, 1), CV_8U, lut), frame);

        if (tracker.update(frame)) {
          if (tracker.getCurrentState() == IkaStateTracker::UNKNOWN &&
              tracker.getBattleCount() > 0) {
            size_t latest = tracker.getBattleCount() - 1;
            IkaStateTracker::BattleInfo info = tracker.getBattleImage(latest);
            cv::imshow("title", info.title);
            cv::imshow("result", info.result);

            static int battle_id = 0;
            handler.PushBattleId(battle_id, info.rule, info.map);
            int name_ids[8];
            for (int i = 0; i < 8; ++i) {
              if (info.status[i] == ImageClipper::VACANCY)
                continue;
              name_ids[i] = name_tracker.GetNameId(info.name_image[i]);
              handler.PushPlayerNameId(info.name_image[i], name_ids[i]);
              handler.PushBattleResult(
                  battle_id, name_ids[i], i, info.kill[i],
                  info.death[i],
                  info.rule == TitlePageReader::NAWABARI ?
                      info.paint_point[i] : -1,
                  info.status[i]);
            }
            battle_id ++;
            handler.MaybeFlush();
          }
        }
      }
    }

    char buf[256];
    double fps = (double)(kBillion) / (double)(fps_interval.diff());
    snprintf(buf, 256, "FPS: %2.1f State: %s", fps, tracker.getStateMessage());
    cv::putText(raw_frame, buf, cv::Point(30,30),
                cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0,255,0), 1, CV_AA);
    cv::imshow("preview", raw_frame);
  }
}
