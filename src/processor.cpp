#include "processor.h"


processor::processor() {
  atomic_thread.store(false);
  buf_in = new short[in_channels * n_hop];
  buf_out = new short[out_channels * n_hop];
}

processor::~processor() {
  delete[] buf_in;
  delete[] buf_out;
}

void processor::Process() {
  if (atomic_thread.load()) {
    printf("Still Processing!!\n");
    return;
  }
  atomic_thread.store(true);

  mpNCObj = mpNC_construct3(L"mpNC.onnx", 1,2);


  rt_output = new RtOutput(device_out, out_channels, sr, sr, n_hop, n_hop);
  rt_output->PrepStream();
  rt_output->Start();
  printf("OutputStream::Start()\n");

  rt_input = new RtInput(device_in, in_channels, sr, n_hop, n_hop);
  printf("==OpenAudioStream==\ndevice : %d\n_channelsannels : %d\nsamplesr : %d\n", device_in, in_channels, sr);
  rt_input->Start();

  double cur_delay = 0.0;
  reset_delay();


  emit(signal_append_log(QString("Start.")));

  auto time_start = std::chrono::high_resolution_clock::now();
  while (rt_input->IsRunning()) {
    if (rt_input->data.stock.load() > n_hop) {
      rt_input->GetBuffer(buf_in);
      // scaling
      for (int i = 0; i < n_hop * in_channels; i++) {
        buf_in[i] = buf_in[i] * multiple;
      }
      //emit(signal_update(buf_in));
      auto t0 = std::chrono::high_resolution_clock::now();
      if(mpNC_on)
        mpNC_process_int16(mpNCObj, buf_in, buf_in);
      auto t1 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = t1 - t0;
      cur_delay = elapsed.count();

      if (cur_delay > 8) {
        std::chrono::duration<double, std::milli> time_total = t1 - time_start;
        elapsed_time = time_total.count();
        cnt_delay++;
        if (cur_delay > max_delay) {
          max_delay = cur_delay;
        }
        sum_delay += cur_delay;
        avg_delay = sum_delay / cnt_delay;

        double elapsed_sec = elapsed_time / 1000.0;

        QString msg = QString::asprintf("[%10.3lfs][%4d]Delay(ms) %6.2lf  | %4.2lf/s | avg %6.2lf | max %6.2lf |",elapsed_sec, cnt_delay, cur_delay, (double)(cnt_delay)/elapsed_sec, avg_delay, max_delay);

        emit(signal_append_log(msg));

      }


      for (int i = 0; i < out_channels; i++) {
        for (int j = 0; j < n_hop; j++) {
          buf_out[j * out_channels + i] = buf_in[j];
        }
      }
      rt_output->AppendQueue(buf_out);
    }
    else
      SLEEP(10);
  }
  atomic_thread.store(false);
  mpNC_release(mpNCObj);
  mpNCObj = nullptr;
}

void processor::Process(const char* path){
  if (atomic_thread.load()) {
    printf("Still Processing!!\n");
    return;
  }
  atomic_thread.store(true);

  wav_input = new WAV();
  wav_input->OpenFile(path);

  while (!wav_input->IsEOF()) {
    wav_input->ReadUnit(buf_in, n_hop * in_channels);
    // scaling
    for (int i = 0; i < n_hop * in_channels; i++) {
      buf_in[i] = buf_in[i] * multiple;
    }
    emit(signal_update(buf_in));
    SLEEP(32);
  }

  delete wav_input;
  atomic_thread.store(false);

}

void processor::slot_toggle() { 

  if (atomic_thread.load()) {
    printf("STOP\n");
    rt_input->Stop();
  }
  else {
    printf("START\n");

    if (atomic_thread.load()) {
      printf("Warnning::Process thread is still running");
      return;
    }
    else {
      if (thread_process) {
        delete thread_process;
        thread_process = nullptr;
      }
    }

    thread_process = new std::thread([=] {this->Process(); });
    thread_process->detach();

  }
}


void processor::slot_load(QString fileName) {
  printf("Load %s\n", fileName.toStdString().c_str());

    if (atomic_thread.load()) {
      printf("Warnning::Process thread is still running");
      return;
    }
    else {
      if (thread_process) {
        delete thread_process;
        thread_process = nullptr;
      }
    }

    thread_process = new std::thread([=] {this->Process(fileName.toStdString().c_str()); });
    thread_process->detach();


}

void processor::Run(){
  

}

/* Delay */
void processor::reset_delay() {
  cnt_delay = 0;
  max_delay = 0.0;
  avg_delay = 0.0;
  sum_delay = 0.0;
  elapsed_time = 0.0;
}


void processor::EnableRealtimeMode(bool flag) {
  if (mpNCObj) {
    if (flag) {
      mpNC_enable_realtime_mode(mpNCObj);
      signal_append_log(QString("Enable mpNC realtime mode"));
    }
    else {
      signal_append_log(QString("Disable mpNC realtime mode"));
      mpNC_disable_realtime_mode(mpNCObj);
    }
  }
  else 
    signal_append_log(QString("Nothing happened."));
}

void processor::EnablempNC(bool flag) {
  mpNC_on = flag;
  if(flag)
    signal_append_log(QString("mpNC On"));
  else
    signal_append_log(QString("mpNC Off"));
}
