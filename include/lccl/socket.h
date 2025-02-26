#ifndef LCCL_INCLUDE_LCCL_SOCK_H_
#define LCCL_INCLUDE_LCCL_SOCK_H_

#if defined(_MSC_VER)
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

#include "lccl.h"

#define LCCL_SOCKET_BEGIN_NAMESPACE namespace skt {
#define LCCL_SOCKET_END_NAMESPACE }

LCCL_BEGIN_NAMESPACE
LCCL_SOCKET_BEGIN_NAMESPACE

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

    virtual bool JoinMulticastGroup(int fd, IAddr *group_addr) = 0;
    virtual bool DropMulticastGroup(int fd, IAddr *group_addr) = 0;

    // 白名单
    virtual bool AddMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;
    virtual bool DropMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;

    // 黑名单
    virtual bool BlockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;
    virtual bool UnblockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr) = 0;
};

LCCL_API void InitEnv();
LCCL_API void DeinitEnv();
LCCL_API int GetLastErrorCode();

LCCL_API std::shared_ptr<IAddr> CreateAddr(const char *host, uint16_t port, bool local);
LCCL_API std::shared_ptr<IAddr> CreateAddr(const sockaddr *sa, bool local);

LCCL_API std::shared_ptr<IAddr> Accept(int fd, int &remote_fd);
LCCL_API std::shared_ptr<IAddr> RecvFrom(int fd, void *buf, int &len);
LCCL_API std::shared_ptr<IAddr> GetSockName(int fd);

LCCL_API int CompareIp(const char *lhs_ip, const char *rhs_ip);
LCCL_API AddrTypes GetIpType(const char *ip);
LCCL_API bool IsIpMulticast(const char *ip);

LCCL_API int CompareSa(const sockaddr *lhs_sa, const sockaddr *rhs_sa);
LCCL_API AddrTypes GetSaType(const sockaddr *sa);
LCCL_API bool IsSaMulticast(const sockaddr *sa);

LCCL_API bool SetBlockMode(int fd, bool block);
LCCL_API bool SetTTL(int fd, int ttl);

LCCL_SOCKET_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_SOCK_H_
