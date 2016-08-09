#include "rtRemoteAsyncHandle.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"

rtRemoteAsyncHandle::rtRemoteAsyncHandle(rtRemoteEnvironment* env, rtRemoteCorrelationKey k)
  : m_env(env)
  , m_key(k)
  , m_error(RT_ERROR_IN_PROGRESS)
{
  m_env->registerResponseHandler(&rtRemoteAsyncHandle::onResponseHandler_Dispatch,
    this, m_key);
}

rtRemoteAsyncHandle::~rtRemoteAsyncHandle()
{
  if (m_key != kInvalidCorrelationKey)
    m_env->removeResponseHandler(m_key);
}

rtError
rtRemoteAsyncHandle::onResponseHandler(std::shared_ptr<rtRemoteClient>& /*client*/,
  rtRemoteMessagePtr const& doc)
{
  m_doc = doc;
  return RT_OK;
}

rtError
rtRemoteAsyncHandle::wait(uint32_t delay)
{
  if (m_error != RT_ERROR_IN_PROGRESS)
    return m_error;

  if (delay == 0)
    delay = m_env->Config->environment_request_timeout();

  if (!m_env->Config->server_use_dispatch_thread())
  {
    auto timeout = std::chrono::system_clock::now() + std::chrono::milliseconds(delay);

    rtRemoteCorrelationKey k = kInvalidCorrelationKey;
    rtError e = m_env->processSingleWorkItem(&k, true, delay);

    if ((e == RT_OK) && (k == m_key))
    {
      m_env->removeResponseHandler(m_key);
      m_key = kInvalidCorrelationKey;
      return RT_OK;
    }
  }

  return m_env->waitForResponse(std::chrono::milliseconds(delay), m_key);
}


void
rtRemoteAsyncHandle::complete(rtRemoteMessagePtr const& doc, rtError e)
{
  m_doc = doc;
  m_error = e;
}

rtRemoteMessagePtr
rtRemoteAsyncHandle::response() const
{
  return m_doc;
}
