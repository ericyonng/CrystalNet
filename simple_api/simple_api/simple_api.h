
#pragma once

#include <simple_api/simple_api_export.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SIMPLE_API_EXPORT int Init();
extern SIMPLE_API_EXPORT void Destroy();
extern SIMPLE_API_EXPORT void SetCppmonitor(void(*MonitorCb)(), int period);

#ifdef __cplusplus
}
#endif