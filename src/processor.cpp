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

  while (rt_input->IsRunning()) {
    if (rt_input->data.stock.load() > n_hop) {
      rt_input->GetBuffer(buf_in);
      // scaling
      for (int i = 0; i < n_hop * in_channels; i++) {
        buf_in[i] = buf_in[i] * multiple;
      }
      //emit(signal_update(buf_in));
      mpNC_process_int16(mpNCObj, buf_in, buf_in);

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

