#pragma once

#if _WIN32
#define CHAR_TYPE wchar_t
  #ifdef __cplusplus
    #ifdef _EXPORT_
      #define DLL_PREFIX __declspec(dllexport)
    #else
      #define DLL_PREFIX  __declspec(dllimport)
    #endif
  #endif
#else
  #define DLL_PREFIX
  #define CHAR_TYPE char
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_PREFIX void* mpNC_construct(const CHAR_TYPE *);
DLL_PREFIX void* mpNC_construct3(const CHAR_TYPE *,int,int);
DLL_PREFIX void mpNC_release(void *obj);

DLL_PREFIX void mpNC_process_int16(void*obj, short *buf_in, short *buf_out);
DLL_PREFIX void mpNC_process_int32(void*obj, int *buf_in, int *buf_out);
DLL_PREFIX void mpNC_process_float32(void*obj, float *buf_in, float *buf_out);
DLL_PREFIX void mpNC_process_float64(void*obj, double *buf_in, double *buf_out);

DLL_PREFIX void mpNC_enable_latency(void* obj, int buffer_size, int initial_delay);
DLL_PREFIX void mpNC_disable_latency(void* obj);
DLL_PREFIX void mpNC_set_VAD_thr(void* obj,double);


DLL_PREFIX void mpNC_enable_realtime_mode(void* obj);
DLL_PREFIX void mpNC_disable_realtime_mode(void* obj);
DLL_PREFIX void mpNC_reset_state(void* obj);

#ifdef __cplusplus
}
#endif





