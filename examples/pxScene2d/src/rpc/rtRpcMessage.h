#ifndef __RT_RPC_MESSAGE_H__
#define __RT_RPC_MESSAGE_H__

#include <limits>
#include <rapidjson/document.h>
#include <rtError.h>

#define kFieldNameMessageType "message.type"
#define kFieldNameCorrelationKey "correlation.key"
#define kFieldNameObjectId "object.id"
#define kFieldNamePropertyName "property.name"
#define kFieldNamePropertyIndex "property.index"
#define kFieldNameStatusCode "status.code"
#define kFieldNameStatusMessage "status.message"
#define kFieldNameFunctionName "function.name"
#define kFieldNameFunctionIndex "function.index"
#define kFieldNameFunctionArgs "function.args"
#define kFieldNameFunctionReturn "function.return_value"
#define kFieldNameValueType "type"
#define kFieldNameValueValue "value"

#define kMessageTypeSetByNameRequest "set.byname.request"
#define kMessageTypeSetByNameResponse "set.byname.response"
#define kMessageTypeSetByIndexRequest "set.byindex.request"
#define kMessageTypeSetByIndexResponse "set.byindex.response"
#define kMessageTypeGetByNameRequest "get.byname.request"
#define kMessageTypeGetByNameResponse "get.byname.response"
#define kMessageTypeGetByIndexRequest "get.byindex.request"
#define kMessageTypeGetByIndexResponse "get.byindex.response"
#define kMessageTypeOpenSessionRequest "session.open.request"
#define kMessageTypeOpenSessionResponse "session.open.response"
#define kMessageTypeMethodCallResponse "method.call.response"
#define kMessageTypeMethodCallRequest "method.call.request"
#define kMessageTypeKeepAliveRequest "keep_alive.request"

#define kInvalidPropertyIndex std::numeric_limits<uint32_t>::max()
#define kInvalidCorrelationKey std::numeric_limits<uint32_t>::max()

char const* rtMessage_GetPropertyName(rapidjson::Document const& doc);
uint32_t    rtMessage_GetPropertyIndex(rapidjson::Document const& doc);
char const* rtMessage_GetMessageType(rapidjson::Document const& doc);
uint32_t    rtMessage_GetCorrelationKey(rapidjson::Document const& doc);
char const* rtMessage_GetObjectId(rapidjson::Document const& doc);

rtError     rtMessage_GetStatusCode(rapidjson::Document const& doc);

#endif
