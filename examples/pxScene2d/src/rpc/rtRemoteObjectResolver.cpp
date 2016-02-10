#include "rtRemoteObjectResolver.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"

#include <rtLog.h>

#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

void
dump_document(rapidjson::Document const& doc)
{
  rapidjson::StringBuffer buff;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
  doc.Accept(writer);
  printf("\n%s\n", buff.GetString());
}

rtRemoteObjectResolver::rtRemoteObjectResolver(sockaddr_storage const& rpc_endpoint)
  : m_mcast_fd(-1)
  , m_ucast_fd(-1)
  , m_ucast_len(0)
  , m_read_thread(0)
  , m_pid(getpid())
  , m_command_handlers()
  , m_rpc_addr()
  , m_rpc_port(0)
  , m_seq_id(0)
{
  memset(&m_mcast_dest, 0, sizeof(m_mcast_dest));
  memset(&m_mcast_src, 0, sizeof(m_mcast_src));
  memset(&m_ucast_endpoint, 0, sizeof(m_ucast_endpoint));

  pthread_mutex_init(&m_mutex, NULL);
  pthread_cond_init(&m_cond, NULL);

  m_command_handlers.insert(cmd_handler_map_t::value_type("search", &rtRemoteObjectResolver::on_search));
  m_command_handlers.insert(cmd_handler_map_t::value_type("locate", &rtRemoteObjectResolver::on_locate));
    
  char addr_buff[128];

  void* addr = NULL;
  rtGetInetAddr(rpc_endpoint, &addr);

  socklen_t len;
  rtSocketGetLength(rpc_endpoint, &len);
  char const* p = inet_ntop(rpc_endpoint.ss_family, addr, addr_buff, len);
  if (p)
    m_rpc_addr = p;

  rtGetPort(rpc_endpoint, &m_rpc_port);
}

rtRemoteObjectResolver::~rtRemoteObjectResolver()
{
  if (m_mcast_fd != -1)
    close(m_mcast_fd);
  if (m_ucast_fd != -1)
    close(m_ucast_fd);
}

rtError
rtRemoteObjectResolver::open(char const* dstaddr, uint16_t port, char const* srcaddr)
{
  rtError err = RT_OK;

  err = rtParseAddress(m_mcast_dest, dstaddr, port);
  if (err != RT_OK)
    return err;

  // TODO: no need to parse srcaddr multiple times, just copy result
  err = rtParseAddress(m_mcast_src, srcaddr, port);
  if (err != RT_OK)
    return err;

  err = rtParseAddress(m_ucast_endpoint, srcaddr, 0);
  if (err != RT_OK)
    return err;

  err = open_unicast_socket();
  if (err != RT_OK)
    return err;

  return RT_OK;
}

rtError
rtRemoteObjectResolver::open_unicast_socket()
{
  int ret = 0;
  int err = 0;

  m_ucast_fd = socket(m_ucast_endpoint.ss_family, SOCK_DGRAM, 0);
  if (m_ucast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ucast_endpoint.ss_family, strerror(err));
    return RT_FAIL;
  }

  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);

  // listen on ANY port
  ret = bind(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to bind unicast endpoint: %s", strerror(err));
    return RT_FAIL;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);
  ret = getsockname(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), &m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to get socketname. %s", strerror(err));
    return RT_FAIL;
  }
  else
  {
    sockaddr_in* saddr = reinterpret_cast<sockaddr_in *>(&m_ucast_endpoint);
    rtLogInfo("local udp socket bound to %s:%d", inet_ntoa(saddr->sin_addr), ntohs(saddr->sin_port));
  }

  // btw, when we use this socket to send multicast, use the right interface
  in_addr mcast_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;
  err = setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char *>(&mcast_interface),
    sizeof(mcast_interface));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set outgoing multicast interface. %s", strerror(err));
    return RT_FAIL;
  }

  #if 0
  char loop = 1;
  if (setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtLogError("failed to disable multicast loopback: %s", strerror(errno));
    return RT_FAIL;
  }
  #endif

  return RT_OK;
}

rtError
rtRemoteObjectResolver::on_search(document_ptr_t const& d, sockaddr_storage const& soc)
{
  rapidjson::Document& doc = *d;

  int pid;
  int seq_id;

  if (doc.HasMember("source"))
  {
    pid = doc["source"].GetInt();
    // did we get our own search reques? If so, just ignore
    if (m_pid == pid)
      return RT_OK;
  }

  if (doc.HasMember(kFieldNameCorrelationKey))
    seq_id = doc[kFieldNameCorrelationKey].GetInt();

  auto itr = m_registered_objects.end();

  std::string const id = doc[kFieldNameObjectId].GetString();
  if (doc.HasMember(kFieldNameObjectId))
  {
    pthread_mutex_lock(&m_mutex);
    itr = m_registered_objects.find(id);
    pthread_mutex_unlock(&m_mutex);
  }

  if (itr != m_registered_objects.end())
  {
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember(kFieldNameMessageType, "locate", doc.GetAllocator());
    doc.AddMember(kFieldNameObjectId, id, doc.GetAllocator());
    doc.AddMember("ip", m_rpc_addr, doc.GetAllocator());
    doc.AddMember("port", m_rpc_port, doc.GetAllocator());
    // echo these back to sender
    doc.AddMember("source", pid, doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, seq_id, doc.GetAllocator());

    return rtSendDocument(doc, m_ucast_fd, &soc);
  }

  return RT_OK;
}

rtError
rtRemoteObjectResolver::on_locate(document_ptr_t const& doc, sockaddr_storage const& /*soc*/)
{
  int seqId = -1;
  if (doc->HasMember(kFieldNameCorrelationKey))
    seqId = (*doc)[kFieldNameCorrelationKey].GetInt();

  pthread_mutex_lock(&m_mutex);
  m_pending_searches[seqId] = doc;
  pthread_cond_signal(&m_cond);
  pthread_mutex_unlock(&m_mutex);

  return RT_OK;
}

void*
rtRemoteObjectResolver::run_listener(void* argp)
{
  rtRemoteObjectResolver* resolver = reinterpret_cast<rtRemoteObjectResolver *>(argp);
  resolver->run_listener();
  return NULL;
}

rtError
rtRemoteObjectResolver::resolveObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout)
{
  rtError err = RT_OK;
  rtAtomic seqId = rtAtomicInc(&m_seq_id);

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember(kFieldNameMessageType, "search", doc.GetAllocator());
  doc.AddMember(kFieldNameObjectId, name, doc.GetAllocator());
  doc.AddMember("source", m_pid, doc.GetAllocator());
  doc.AddMember(kFieldNameCorrelationKey, seqId, doc.GetAllocator());

  err = rtSendDocument(doc, m_ucast_fd, &m_mcast_dest);
  if (err != RT_OK)
    return err;

  document_ptr_t search_response;
  request_map_t::const_iterator itr;

  // wait here until timeout expires or we get a response that matches out pid/seqid
  pthread_mutex_lock(&m_mutex);
  while ((itr = m_pending_searches.find(seqId)) == m_pending_searches.end())
    pthread_cond_wait(&m_cond, &m_mutex);
  if (itr != m_pending_searches.end())
  {
    search_response = itr->second;
    m_pending_searches.erase(itr);
  }
  pthread_mutex_unlock(&m_mutex);

  if (!search_response)
    return RT_FAIL;

  // response is in itr
  if (search_response)
  {
    assert(search_response->HasMember("ip"));
    assert(search_response->HasMember("port"));

    rtError err = rtParseAddress(endpoint, (*search_response)["ip"].GetString(),
      (*search_response)["port"].GetInt());

    if (err != RT_OK)
      return err;
  }

  return RT_OK;
}

void
rtRemoteObjectResolver::run_listener()
{
  buff_t buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_mcast_fd, &maxFd);
    rtPushFd(&read_fds, m_ucast_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_mcast_fd, &maxFd);
    rtPushFd(&err_fds, m_ucast_fd, &maxFd);

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed: %s", strerror(err));
      continue;
    }

    if (FD_ISSET(m_mcast_fd, &read_fds))
      do_read(m_mcast_fd, buff);

    if (FD_ISSET(m_ucast_fd, &read_fds))
      do_read(m_ucast_fd, buff);
  }
}

void
rtRemoteObjectResolver::do_read(int fd, buff_t& buff)
{
  // we only suppor v4 right now. not sure how recvfrom supports v6 and v4
  sockaddr_storage src;
  socklen_t len = sizeof(sockaddr_in);

  #if 0
  ssize_t n = read(m_mcast_fd, &m_read_buff[0], m_read_buff.capacity());
  #endif

  ssize_t n = recvfrom(fd, &buff[0], buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    do_dispatch(&buff[0], static_cast<int>(n), &src);
}

void
rtRemoteObjectResolver::do_dispatch(char const* buff, int n, sockaddr_storage* peer)
{
  // rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  // printf("read: %d\n", int(n));
  #ifdef RT_RPC_DEBUG
  rtLogDebug("read:\n\t\"%.*s\"\n", n, buff); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);
  #endif

  std::shared_ptr<rapidjson::Document> doc(new rapidjson::Document());

  rapidjson::MemoryStream stream(buff, n);
  if (doc->ParseStream<rapidjson::kParseDefaultFlags>(stream).HasParseError())
  {
    int begin = doc->GetErrorOffset();
    int end = begin + 16;
    if (end > n)
      end = n;
    int length = (end - begin);

    rtLogWarn("unparsable JSON read: %d", doc->GetParseError());
    rtLogWarn("\"%.*s\"\n", length, buff);
  }
  else
  {
    if (!doc->HasMember(kFieldNameMessageType))
    {
      rtLogWarn("recived JSON payload without type");
      return;
    }

    std::string cmd = (*doc)[kFieldNameMessageType].GetString();

    auto itr = m_command_handlers.find(cmd);
    if (itr == m_command_handlers.end())
    {
      rtLogWarn("no command handler registered for: %s", cmd.c_str());
      return;
    }

    // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
    #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

    rtError err = CALL_MEMBER_FN(*this, itr->second)(doc, *peer);
    if (err != RT_OK)
    {
      rtLogWarn("failed to run command for %s. %d", cmd.c_str(), err);
      return;
    }
  }
}

rtError
rtRemoteObjectResolver::start()
{
  rtError err = open_multicast_socket();
  if (err != RT_OK)
    return err;

  pthread_create(&m_read_thread, NULL, &rtRemoteObjectResolver::run_listener, this);
  return RT_OK;
}

rtError
rtRemoteObjectResolver::registerObject(std::string const& name)
{
  pthread_mutex_lock(&m_mutex);
  m_registered_objects.insert(name);
  pthread_mutex_unlock(&m_mutex);
  return RT_OK;
}

rtError
rtRemoteObjectResolver::open_multicast_socket()
{
  int err = 0;

  m_mcast_fd = socket(m_mcast_dest.ss_family, SOCK_DGRAM, 0);
  if (m_mcast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create multicast socket. %s", strerror(err));
    return RT_FAIL;
  }

  // re-use because multiple applications may want to join group on same machine
  int reuse = 1;
  err = setsockopt(m_mcast_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set reuseaddr. %s", strerror(err));
    return RT_FAIL;
  }

  sockaddr_in saddr = *(reinterpret_cast<sockaddr_in *>(&m_mcast_src));
  saddr.sin_addr.s_addr = INADDR_ANY;
  err = bind(m_mcast_fd, reinterpret_cast<sockaddr *>(&saddr), sizeof(sockaddr_in));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to bind socket. %s", strerror(err));
    return RT_FAIL;
  }

  // join group
  ip_mreq group;
  group.imr_multiaddr = reinterpret_cast<sockaddr_in *>(&m_mcast_dest)->sin_addr;
  group.imr_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;

  err = setsockopt(m_mcast_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to join mcast group. %s", strerror(err));
    return RT_FAIL;
  }

  rtLogInfo("successfully joined multicast group: %s on interface: %s",
      inet_ntoa(group.imr_multiaddr), inet_ntoa(group.imr_interface));

  return RT_OK;
}
