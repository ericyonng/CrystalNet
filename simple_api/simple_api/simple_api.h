
#pragma once

#include <simple_api/simple_api_export.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SIMPLE_API_EXPORT int Init();
extern SIMPLE_API_EXPORT void Destroy();
extern SIMPLE_API_EXPORT void SetCppmonitor(void(*MonitorCb)(), int period);
extern SIMPLE_API_EXPORT void Log(const char *content, Int32 contentSize);
extern SIMPLE_API_EXPORT void PushProfile(int messageId, Int64 dispatchMs, Int64 fromReqUtilResponse);

#ifdef __cplusplus
}
#endif