
#pragma once

#include <simple_api/simple_api_export.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SIMPLE_API_EXPORT int Init(bool needSignalHandle);
extern SIMPLE_API_EXPORT void Destroy();
extern SIMPLE_API_EXPORT void SetCppmonitor(void(*MonitorCb)(), int period);
extern SIMPLE_API_EXPORT void Log(const char *content, Int32 contentSize);
extern SIMPLE_API_EXPORT void PushProfile(Int64 nowTime, int messageId, int requestId, Int64 dispatchMs
, Int64 gwRecvRequestTime, Int64 gwSendRequestTime
, Int64 gsRecvRequestTime, Int64 gsHandlerRequestTime
, Int64 gsSendResponseTime, Int64 gwRecvResponseTime);

#ifdef __cplusplus
}
#endif