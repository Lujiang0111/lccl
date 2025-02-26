#ifndef LCCL_INTERNAL_SOCKET_V6_ADDR_H_
#define LCCL_INTERNAL_SOCKET_V6_ADDR_H_

#if defined(_MSC_VER)
#include <ws2ipdef.h>
#else
#include <netinet/in.h>
#endif

#include <string>
#include "lccl/socket.h"

LCCL_BEGIN_NAMESPACE
LCCL_SOCKET_BEGIN_NAMESPACE

class V6Addr : public IAddr
{
public:
    V6Addr();
    virtual ~V6Addr();

    bool Init(const char *ip, uint16_t port, bool local);
    bool Init(const sockaddr *sa, bool local);

    virtual sockaddr *GetNative();
    virtual AddrTypes GetType() const;
    virtual bool IsMulticast() const;
    virtual const char *GetIp() const;
    virtual uint16_t GetPort() const;
    virtual const char *GetDev() const;

    virtual int Compare(IAddr *rhs);

    virtual bool JoinMulticastGroup(int fd, IAddr *group_addr);
    virtual bool DropMulticastGroup(int fd, IAddr *group_addr);

    virtual bool AddMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr);
    virtual bool DropMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr);

    virtual bool BlockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr);
    virtual bool UnblockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr);

private:
    bool ParseSa(bool local);

private:
    std::string ip_;
    uint16_t port_;
    sockaddr_in6 sa6_;

    std::string dev_;
    int dev_num_;
    int scope_id_;
};

LCCL_SOCKET_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INTERNAL_SOCKET_V6_ADDR_H_
