
#pragma once

#include <simple_api/simple_api_export.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SIMPLE_API_EXPORT int SimpleApiInit(bool needSignalHandle);
extern SIMPLE_API_EXPORT void SimpleApiDestroy();
extern SIMPLE_API_EXPORT void SimpleApiSetCppmonitor(void(*MonitorCb)(), int period);
extern SIMPLE_API_EXPORT void SimpleApiLog(const char *content, Int32 contentSize);
extern SIMPLE_API_EXPORT void SimpleApiPushProfile(Int64 nowTime, int messageId, int requestId, Int64 dispatchMs
, Int64 gwRecvRequestTime, Int64 gwSendRequestTime, Int64 gwPrepareTurnRequestToRpcTime
, Int64 gsRecvRequestTime, Int64 gsDispatchRequestTime, Int64 gsHandlerRequestTime
, Int64 gsSendResponseTime, Int64 gwRecvResponseTime);

extern SIMPLE_API_EXPORT void SimpleApiPushProfile2(Int64 nowTime, int messageId, int requestId, Int64 dispatchMs
, Int64 gwRecvMsgTime, Int64 gwPrepareRpcTime, Int64 gsRecvRpcTime
, Int64 gsDispatchMsgTime, Int64 gsHandledTime);

extern SIMPLE_API_EXPORT void SimpleApiPushProfile3(Int64 nowTime, int messageId, int requestId
, Int64 gwRecvMsgTime, Int64 gwPrepareRpcTime, Int64 gsRecvRpcTime
, Int64 gsDispatchMsgTime, Int64 asynOverDispatchTime);

#ifdef __cplusplus
}
#endif