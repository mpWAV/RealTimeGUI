#include <QObject>

#include "WAV.h"
#include "STFT.h"
#include "RtInput.h"
#include "RtOutput.h"

#include <vector>
#include <atomic>
#include <thread>

#include <time.h>

#include "mpNC.h"

using std::vector;

class processor : public QObject {
  Q_OBJECT
private:
  bool isRunning = false;

  RtInput* rt_input = nullptr;
  RtOutput* rt_output = nullptr;
  WAV* wav_input = nullptr;
  void* mpNCObj = nullptr;

  short* buf_in = nullptr;
  short* buf_out = nullptr;

  std::thread* thread_process = nullptr;
  std::atomic<bool> atomic_thread; // is thread running ?

  void Process();
  void Process(const char* path);

public:
  int device_in;
  int device_out;

  processor();
  ~processor();

  /* param */

  int in_channels = 1;
  const int out_channels = 2;
  int sr = 16000;
  int n_fft = 512;
  int n_hop = 128;
  double multiple = 1.0;

  int cnt = 0;

  void Run();

  void slot_toggle();
  void slot_load(QString);

signals:
  void signal_update(short*);
  void signal_process_done(const char*);


};
