#ifndef __RT_RPC_STREAM_H__
#define __RT_RPC_STREAM_H__

#include "rtRpcTypes.h"
#include "rtSocketUtils.h"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

class rtRpcClient;
class rtRpcMessage;
class rtRpcRequest;
class rtRpcStreamSelector;

class rtRpcStream : public std::enable_shared_from_this<rtRpcStream>
{
  friend class rtRpcStreamSelector;
  friend class rtRpcClient;

public:
  using MessageHandler = std::function<rtError (rtJsonDocPtr const& doc)>;

  rtRpcStream(int fd, sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint);
  ~rtRpcStream();

  rtRpcStream(rtRpcStream const&) = delete;
  rtRpcStream& operator = (rtRpcStream const&) = delete;

  inline bool isConnected() const
    { return m_fd != -1; }

  rtError open();
  rtError close();
  rtError connectTo(sockaddr_storage const& endpoint);
  rtError connect();
  rtError send(rtRpcMessage const& msg);
  rtError sendRequest(rtRpcRequest const& req, MessageHandler handler, uint32_t timeout = 1000);
  rtError setMessageCallback(MessageHandler handler);
  rtError setInactivityCallback(rtRpcInactivityHandler handler);

  inline sockaddr_storage getLocalEndpoint() const
    { return m_local_endpoint; }

  inline sockaddr_storage getRemoteEndpoint() const
    { return m_remote_endpoint; }

private:
  rtError sendDocument(rapidjson::Document const& doc)
  {
    return rtSendDocument(doc, m_fd, nullptr);
  }

  rtError onIncomingMessage(rtSocketBuffer& buff, time_t now);
  rtError onInactivity(time_t now);
  rtJsonDocPtr waitForResponse(int key, uint32_t timeout);

private:
  using rtRequestMap = std::map< rtCorrelationKey, rtJsonDocPtr >;

  int 				m_fd;
  time_t 			m_last_message_time;
  MessageHandler		m_message_handler;
  rtRpcInactivityHandler	m_inactivity_handler;
  std::mutex			m_mutex;
  std::condition_variable	m_cond;
  rtRequestMap			m_requests;
  sockaddr_storage		m_local_endpoint;
  sockaddr_storage		m_remote_endpoint;
};

#endif
