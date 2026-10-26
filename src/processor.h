#include <QObject>

#include "WAV.h"
#include "STFT.h"
#include "RtInput.h"
#include "RtOutput.h"

#include <chrono>

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
  short* buf_tmp = nullptr;

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
  const int out_channels = 1;
  int sr = 16000;
  int n_fft = 512;
  // mpNCw_process_int16() 이 정확히 160 샘플을 읽고 160 샘플을 쓴다
  // (mpNCw_decl.h 의 n_hop_in / n_hop_out, const 이고 인자가 아니다).
  // 이 값이 160 이 아니면 블록 앞 160 개만 처리되고 뒤쪽은 손대지 않은 값이
  // 남아 조용히 소리가 깨진다. mpNCw 쪽이 바뀌지 않는 한 고정이다.
  // 입출력 버퍼, RtInput / RtOutput 프레임 크기가 모두 이 값에서 나온다.
  int n_hop = 160; // was 256, mpNC_* 를 직접 부르던 시절의 값
  double multiple = 1.0;

  int cnt = 0;

  void Run();

  void slot_toggle();
  void slot_load(QString);

signals:
  void signal_update(short*);
  void signal_process_done(const char*);


  /* Delay */
private : 
  int cnt_delay = 0;
  double max_delay = 0.0;
  double avg_delay = 0.0;
  double sum_delay = 0.0;
  double elapsed_time = 0.0;

  bool mpNC_on = false;

  void reset_delay();

signals : 
  void signal_append_log(QString);

public :
  void EnablempNC(bool);
  void EnableRealtimeMode(bool enable);



};
