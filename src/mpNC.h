#pragma once

// -----------------------------------------------------------------------------
//  mpNCw API - IR_Link_mpNC.dll 이 내보내는 유일한 API 집합.
//
//  고객이 받는 package/mpNC.h 와 같은 함수들이다. 이 GUI 의 목적이 mpNCw 를
//  실시간으로 테스트하는 것이므로 제품과 똑같은 경로를 타야 한다.
//    팝 필터 -> 레벨 가드 -> 160/256 재블록 -> mpNC -> 레벨 복원
//
//  이전에는 mpNC_* 를 직접 불렀는데, 그 경로는 mpNC 클래스를 날것으로 노출해
//  위 처리를 전부 건너뛰었고 라이선스 검사도 없었다. 그래서 DLL 에서 그 래퍼를
//  제거했다 (2025_irlink_mpNCw/CMakeLists.txt 의 SRC 주석 참조).
//
//  ** 블록 크기는 160 샘플 / 16 kHz 고정 **
//  mpNCw_decl.h 의 n_hop_in / n_hop_out 이 const 160 이고 인자가 아니다.
//  processor.h 의 n_hop 과 반드시 일치해야 한다. 어긋나면 앞 160 개만 처리되고
//  뒤쪽은 손대지 않은 값이 남아 조용히 소리가 깨진다.
//
//  모델은 mpNCw 가 실행 디렉터리의 mpNC_Lite.onnx 를 자동으로 읽는다. 경로
//  인자가 없으므로 bin/mpNC.onnx 는 이 경로에서 쓰이지 않는다.
// -----------------------------------------------------------------------------

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

DLL_PREFIX void* mpNCw_construct();
DLL_PREFIX void mpNCw_release(void *obj);

// buf_in / buf_out 은 각각 160 샘플.
DLL_PREFIX void mpNCw_process_int16(void*obj, short *buf_in, short *buf_out);
DLL_PREFIX void mpNCw_process_int32(void*obj, int *buf_in, int *buf_out);
DLL_PREFIX void mpNCw_process_float32(void*obj, float *buf_in, float *buf_out);
DLL_PREFIX void mpNCw_process_float64(void*obj, double *buf_in, double *buf_out);

// true 면 고정 지연 모드 (내부 버퍼 4096 / 초기 지연 2048 샘플 = 128 ms).
// 실시간 모니터링에서는 켜지 말 것.
DLL_PREFIX void mpNCw_set_latency_mode(void* obj, bool flag);
DLL_PREFIX void mpNCw_set_VAD_threshold(void* obj, double thr);
DLL_PREFIX void mpNCw_set_latency_size(void* obj, int buffer_size, int initial_delay);

// 이전 mpNC_* 에 있었으나 mpNCw 에 대응이 없는 것들:
//   mpNC_construct / mpNC_construct3  -> mpNCw_construct() (모델 경로 지정 불가)
//   mpNC_enable_latency               -> mpNCw_set_latency_size
//   mpNC_disable_latency              -> mpNCw_set_latency_mode(obj, false)
//   mpNC_set_VAD_thr                  -> mpNCw_set_VAD_threshold
//   mpNC_enable_realtime_mode         -> 없음
//   mpNC_disable_realtime_mode        -> 없음
//   mpNC_reset_state                  -> 없음 (해제 후 재생성으로 대체)

#ifdef __cplusplus
}
#endif
