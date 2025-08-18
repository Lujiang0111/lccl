#ifndef LCCL_INTERNAL_SOCKET_V6_ADDR_H_
#define LCCL_INTERNAL_SOCKET_V6_ADDR_H_

#if defined(_MSC_VER)
#include <ws2ipdef.h>
#else
#include <netinet/in.h>
#endif

#include <string>
#include "lccl/socket.h"

LCCL_NAMESPACE_BEGIN
LCCL_SOCKET_NAMESPACE_BEGIN

class V6Addr : public IAddr
{
public:
    V6Addr();
    virtual ~V6Addr();

    bool Init(const char *ip, uint16_t port, bool local);
    bool Init(const sockaddr *sa, bool local);
    bool Init(const char *dev);

    virtual sockaddr *GetNative();
    virtual AddrTypes GetType() const;
    virtual bool IsMulticast() const;
    virtual const char *GetIp() const;
    virtual uint16_t GetPort() const;
    virtual const char *GetDev() const;

    virtual int Compare(IAddr *rhs);

    virtual int Bind(int fd);
    virtual int Connect(int fd);
    virtual int MulticastIf(int fd);

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

LCCL_SOCKET_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INTERNAL_SOCKET_V6_ADDR_H_
