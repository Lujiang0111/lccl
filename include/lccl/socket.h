#ifndef LCCL_INCLUDE_LCCL_SOCK_H_
#define LCCL_INCLUDE_LCCL_SOCK_H_

#if defined(_MSC_VER)
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

#include "lccl.h"

#define LCCL_SOCKET_NAMESPACE_BEGIN namespace skt {
#define LCCL_SOCKET_NAMESPACE_END }

LCCL_NAMESPACE_BEGIN
LCCL_SOCKET_NAMESPACE_BEGIN

enum class AddrTypes
{
    kUnknown = 0,
    kIpv4,
    kIpv6,
};

class IAddr
{
public:
    virtual ~IAddr() = default;

    virtual sockaddr *GetNative() = 0;
    virtual AddrTypes GetType() const = 0;
    virtual bool IsMulticast() const = 0;
    virtual const char *GetIp() const = 0;
    virtual uint16_t GetPort() const = 0;
    virtual const char *GetDev() const = 0;

    virtual int Compare(IAddr *rhs) = 0;

    virtual int Bind(int fd) = 0;
    virtual int Connect(int fd) = 0;
    virtual int MulticastIf(int fd) = 0;

    // 组播
    virtual bool JoinMulticastGroup(int fd, IAddr *group_addr) = 0;
    virtual bool DropMulticastGroup(int fd, IAddr *group_addr) = 0;

    // 白名单
    virtual bool AddMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;
    virtual bool DropMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;

    // 黑名单
    virtual bool BlockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;
    virtual bool UnblockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;
};

LCCL_API int GetLastErrorCode();

LCCL_API std::shared_ptr<IAddr> CreateAddr(const char *host, uint16_t port, bool local);
LCCL_API std::shared_ptr<IAddr> CreateAddr(const sockaddr *sa, bool local);
LCCL_API std::shared_ptr<IAddr> CreateLocalAddr(const char *dev, AddrTypes addr_type);

LCCL_API std::shared_ptr<IAddr> Accept(int fd, int &remote_fd);
LCCL_API std::shared_ptr<IAddr> RecvFrom(int fd, void *buf, int &len);
LCCL_API std::shared_ptr<IAddr> GetSockName(int fd);

LCCL_API int CompareIp(const char *lhs_ip, const char *rhs_ip);
LCCL_API AddrTypes GetIpType(const char *ip);
LCCL_API bool IsIpMulticast(const char *ip);

// 只比较sa的IP部分
LCCL_API int CompareSa(const sockaddr *lhs_sa, const sockaddr *rhs_sa);
LCCL_API AddrTypes GetSaType(const sockaddr *sa);
LCCL_API bool IsSaMulticast(const sockaddr *sa);

LCCL_API bool IpHexToStr(AddrTypes addr_type, const void *ip_hex, char *str);

LCCL_API bool SetBlockMode(int fd, bool block);
LCCL_API bool SetTTL(int fd, int ttl);

/** 为socket设置keep alive
* on：是否启用keepalive
* idle：空闲多久后开始发送keepalive探测（秒）
* interval：探测包之间的时间间隔（秒）
* count：未收到响应后发送探测包的次数
*/
LCCL_API bool SetKeepAlive(int fd, bool on, int idle, int interval, int count);

LCCL_SOCKET_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_SOCK_H_
