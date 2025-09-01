#if defined(_MSC_VER)
#include <ws2tcpip.h>
#include <mstcpip.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include "lccl/socket.h"
#include "log/lib_log.h"
#include "socket/v4_addr.h"
#include "socket/v6_addr.h"

LCCL_NAMESPACE_BEGIN
LCCL_SOCKET_NAMESPACE_BEGIN

int GetLastErrorCode()
{
#if defined(_MSC_VER)
    return WSAGetLastError();
#else
    return errno;
#endif
}

std::shared_ptr<IAddr> CreateAddr(const char *host, uint16_t port, bool local)
{
    std::shared_ptr<IAddr> addr = nullptr;
    AddrTypes addr_type = GetIpType(host);
    switch (addr_type)
    {
    case AddrTypes::kIpv4:
    {
        std::shared_ptr<V4Addr> v4_addr = std::make_shared<V4Addr>();
        if (!v4_addr->Init(host, port, local))
        {
            return nullptr;
        }

        addr = v4_addr;
        break;
    }

    case AddrTypes::kIpv6:
    {
        std::shared_ptr<V6Addr> v6_addr = std::make_shared<V6Addr>();
        if (!v6_addr->Init(host, port, local))
        {
            return nullptr;
        }

        addr = v6_addr;
        break;
    }

    default:
    {
        std::string port_str = std::to_string(port);
        struct addrinfo hints = { 0 };
        struct addrinfo *res = nullptr;
        if (0 != getaddrinfo(host, port_str.c_str(), &hints, &res))
        {
            LIB_LOG(lccl::log::Levels::kError, "getaddrinfo fail! host={}, port={}, error={}",
                host, port, GetLastErrorCode());
            break;
        }

        for (struct addrinfo *node = res; node; node = node->ai_next)
        {
            addr = CreateAddr(node->ai_addr, local);
            if (addr)
            {
                break;
            }
        }
        freeaddrinfo(res);
        break;
    }
    }

    return addr;
}

std::shared_ptr<IAddr> CreateAddr(const sockaddr *sa, bool local)
{
    std::shared_ptr<IAddr> addr = nullptr;
    switch (sa->sa_family)
    {
    case AF_INET:
    {
        std::shared_ptr<V4Addr> v4_addr = std::make_shared<V4Addr>();
        if (!v4_addr->Init(sa, local))
        {
            break;
        }

        addr = v4_addr;
        break;
    }

    case AF_INET6:
    {
        std::shared_ptr<V6Addr> v6_addr = std::make_shared<V6Addr>();
        if (!v6_addr->Init(sa, local))
        {
            break;
        }

        addr = v6_addr;
        break;
    }

    default:
        break;
    }

    return addr;
}

std::shared_ptr<IAddr> CreateLocalAddr(const char *dev, AddrTypes addr_type)
{
    if (!dev)
    {
        return nullptr;
    }

    std::shared_ptr<IAddr> addr = nullptr;
    switch (addr_type)
    {
    case AddrTypes::kIpv4:
    {
        std::shared_ptr<V4Addr> v4_addr = std::make_shared<V4Addr>();
        if (!v4_addr->Init(dev))
        {
            break;
        }

        addr = v4_addr;
        break;
    }

    case AddrTypes::kIpv6:
    {
        std::shared_ptr<V6Addr> v6_addr = std::make_shared<V6Addr>();
        if (!v6_addr->Init(dev))
        {
            break;
        }

        addr = v6_addr;
        break;
    }

    default:
        break;
    }

    return addr;
}

std::shared_ptr<IAddr> Accept(int fd, int &remote_fd)
{
    sockaddr_storage remote_sa = { 0 };
    socklen_t remote_addr_len = sizeof(remote_sa);
    remote_fd = static_cast<int>(accept(fd, reinterpret_cast<sockaddr *>(&remote_sa), &remote_addr_len));
    if (remote_fd < 0)
    {
        return nullptr;
    }

    return CreateAddr(reinterpret_cast<sockaddr *>(&remote_sa), false);
}

std::shared_ptr<IAddr> RecvFrom(int fd, void *buf, int &len)
{
    sockaddr_storage remote_sa = { 0 };
    socklen_t remote_sa_len = sizeof(remote_sa);
    len = recvfrom(fd, static_cast<char *>(buf), len, 0, reinterpret_cast<struct sockaddr *>(&remote_sa), &remote_sa_len);
    if (len <= 0)
    {
        return nullptr;
    }

    return CreateAddr(reinterpret_cast<sockaddr *>(&remote_sa), false);
}

std::shared_ptr<IAddr> GetSockName(int fd)
{
    sockaddr_storage remote_sa = { 0 };
    socklen_t remote_sa_len = sizeof(remote_sa);
    if (getsockname(fd, reinterpret_cast<sockaddr *>(&remote_sa), &remote_sa_len) < 0)
    {
        return nullptr;
    }

    return CreateAddr(reinterpret_cast<sockaddr *>(&remote_sa), false);
}

int CompareIp(const char *lhs_ip, const char *rhs_ip)
{
    std::shared_ptr<IAddr> lhs_addr = CreateAddr(lhs_ip, 0, false);
    std::shared_ptr<IAddr> rhs_addr = CreateAddr(rhs_ip, 0, false);
    if ((!lhs_addr) || (!rhs_addr))
    {
        return -2;
    }

    return CompareSa(lhs_addr->GetNative(), rhs_addr->GetNative());
}

AddrTypes GetIpType(const char *ip)
{
    // Ipv6为128位地址，需要16字节大小储存
    uint8_t buf[16] = { 0 };
    if (inet_pton(AF_INET, ip, buf) > 0)
    {
        return AddrTypes::kIpv4;
    }
    else if (inet_pton(AF_INET6, ip, buf) > 0)
    {
        return AddrTypes::kIpv6;
    }

    return AddrTypes::kUnknown;
}

bool IsIpMulticast(const char *ip)
{
    std::shared_ptr<IAddr> addr = CreateAddr(ip, 0, false);
    if (!addr)
    {
        return false;
    }

    return addr->IsMulticast();
}

int CompareSa(const sockaddr *lhs_sa, const sockaddr *rhs_sa)
{
    if (lhs_sa->sa_family != rhs_sa->sa_family)
    {
        return -2;
    }

    switch (lhs_sa->sa_family)
    {
    case AF_INET:
    {
        const sockaddr_in *lhs_sa4 = reinterpret_cast<const sockaddr_in *>(lhs_sa);
        const sockaddr_in *rhs_sa4 = reinterpret_cast<const sockaddr_in *>(rhs_sa);
        for (int index = 0; index < 4; index++)
        {
            if ((reinterpret_cast<const uint8_t *>(&(lhs_sa4->sin_addr.s_addr)))[index] >
                (reinterpret_cast<const uint8_t *>(&(rhs_sa4->sin_addr.s_addr)))[index])
            {
                return 1;
            }
            else if ((reinterpret_cast<const uint8_t *>(&(lhs_sa4->sin_addr.s_addr)))[index] <
                (reinterpret_cast<const uint8_t *>(&(rhs_sa4->sin_addr.s_addr)))[index])
            {
                return -1;
            }
        }
        break;
    }

    case AF_INET6:
    {
        const sockaddr_in6 *lhs_sa6 = reinterpret_cast<const sockaddr_in6 *>(lhs_sa);
        const sockaddr_in6 *rhs_sa6 = reinterpret_cast<const sockaddr_in6 *>(rhs_sa);
        for (int index = 0; index < 16; index++)
        {
            if ((reinterpret_cast<const uint8_t *>(&(lhs_sa6->sin6_addr)))[index] >
                (reinterpret_cast<const uint8_t *>(&(rhs_sa6->sin6_addr)))[index])
            {
                return 1;
            }
            else if ((reinterpret_cast<const uint8_t *>(&(lhs_sa6->sin6_addr)))[index] <
                (reinterpret_cast<const uint8_t *>(&(rhs_sa6->sin6_addr)))[index])
            {
                return -1;
            }
        }
        break;
    }

    default:
        return -3;
    }

    return 0;
}

AddrTypes GetSaType(const sockaddr *sa)
{
    switch (sa->sa_family)
    {
    case AF_INET:
        return AddrTypes::kIpv4;

    case AF_INET6:
        return AddrTypes::kIpv6;

    default:
        break;
    }

    return AddrTypes::kUnknown;
}

bool IsSaMulticast(const sockaddr *sa)
{
    switch (sa->sa_family)
    {
    case AF_INET:
    {
        const sockaddr_in *sa4 = reinterpret_cast<const sockaddr_in *>(sa);
        return IN_MULTICAST(ntohl(sa4->sin_addr.s_addr));
    }

    case AF_INET6:
    {
        const sockaddr_in6 *sa6 = reinterpret_cast<const sockaddr_in6 *>(sa);
        return IN6_IS_ADDR_MULTICAST(&(sa6->sin6_addr));
    }

    default:
        break;
    }

    return false;
}

bool IpHexToStr(AddrTypes addr_type, const void *ip_hex, char *str)
{
    switch (addr_type)
    {
    case lccl::skt::AddrTypes::kIpv4:
        inet_ntop(AF_INET, ip_hex, str, INET_ADDRSTRLEN);
        break;
    case lccl::skt::AddrTypes::kIpv6:
        inet_ntop(AF_INET6, ip_hex, str, INET6_ADDRSTRLEN);
        break;
    default:
        return false;
    }

    return true;
}

bool SetBlockMode(int fd, bool block)
{
#if defined(_MSC_VER)
    u_long argp = (block) ? 0 : 1;
    return (0 == ioctlsocket(fd, FIONBIO, &argp));
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags)
    {
        LIB_LOG(lccl::log::Levels::kError, "Unable to get fd mode, error:{}", GetLastErrorCode());
        return false;
    }

    flags = (block) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (0 == fcntl(fd, F_SETFL, flags));
#endif
}

bool SetTTL(int fd, int ttl)
{
    setsockopt(fd, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl));
    setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (const char *)&ttl, sizeof(ttl));
    setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char *)&ttl, sizeof(ttl));
    return true;
}

bool SetKeepAlive(int fd, bool on, int idle, int interval, int count)
{
#ifdef _MSC_VER
    DWORD bytes_returned;
    tcp_keepalive tk;
    tk.onoff = (on) ? 1 : 0;
    tk.keepalivetime = idle * 1000;
    tk.keepaliveinterval = interval * 1000;

    if (SOCKET_ERROR == WSAIoctl(fd, SIO_KEEPALIVE_VALS, &tk, sizeof(tk),
        nullptr, 0, &bytes_returned, nullptr, nullptr))
    {
        LIB_LOG(lccl::log::Levels::kError, "Failed to set keepalive: {}", GetLastErrorCode());
        return false;
    }

    return true;
#else
    int on_off = (on) ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on_off, sizeof(on_off)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "Failed to set SO_KEEPALIVE, error={}", GetLastErrorCode());
        return false;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "Failed to set TCP_KEEPIDLE, error={}", GetLastErrorCode());
        return false;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0) {
        LIB_LOG(lccl::log::Levels::kError, "Failed to set TCP_KEEPINTVL, error={}", GetLastErrorCode());
        return false;
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "Failed to set TCP_KEEPCNT, error={}", GetLastErrorCode());
        return false;
    }

    return true;
#endif
}

LCCL_SOCKET_NAMESPACE_END
LCCL_NAMESPACE_END
