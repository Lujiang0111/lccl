#if defined(_MSC_VER)
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif

#include <vector>
#include "log/lib_log.h"
#include "socket/v6_addr.h"

LCCL_BEGIN_NAMESPACE
LCCL_SOCKET_BEGIN_NAMESPACE

V6Addr::V6Addr() :
    port_(0),
    sa6_{0},
    dev_num_(0),
    scope_id_(0)
{

}

V6Addr::~V6Addr()
{

}

bool V6Addr::Init(const char *ip, uint16_t port, bool local)
{
    memset(&sa6_, 0, sizeof(sa6_));
    sa6_.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip, sa6_.sin6_addr.s6_addr);
    sa6_.sin6_port = htons(port);

    return ParseSa(local);
}

bool V6Addr::Init(const sockaddr *sa, bool local)
{
    memcpy(&sa6_, sa, sizeof(sa6_));
    return ParseSa(local);
}

sockaddr *V6Addr::GetNative()
{
    return reinterpret_cast<sockaddr *>(&sa6_);
}

AddrTypes V6Addr::GetType() const
{
    return AddrTypes::kIpv6;
}

bool V6Addr::IsMulticast() const
{
    return IN6_IS_ADDR_MULTICAST(&(sa6_.sin6_addr));
}

const char *V6Addr::GetIp() const
{
    return ip_.c_str();
}

uint16_t V6Addr::GetPort() const
{
    return port_;
}

const char *V6Addr::GetDev() const
{
    return dev_.c_str();
}

int V6Addr::Compare(IAddr *rhs)
{
    return CompareSa(GetNative(), rhs->GetNative());
}

bool V6Addr::JoinMulticastGroup(int fd, IAddr *group_addr)
{
    V6Addr *group_v6_addr = dynamic_cast<V6Addr *>(group_addr);
    if (!group_v6_addr)
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr and local addr");
        return false;
    }

    struct ipv6_mreq mreq6;
    mreq6.ipv6mr_multiaddr = group_v6_addr->sa6_.sin6_addr;
    mreq6.ipv6mr_interface = dev_num_;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IPV6_ADD_MEMBERSHIP fail! group ip={}, local ip={}, error={}",
            group_v6_addr->ip_.c_str(), ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V6Addr::DropMulticastGroup(int fd, IAddr *group_addr)
{
    V6Addr *group_v6_addr = dynamic_cast<V6Addr *>(group_addr);
    if (!group_v6_addr)
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr and local addr");
        return false;
    }

    struct ipv6_mreq mreq6;
    mreq6.ipv6mr_multiaddr = group_v6_addr->sa6_.sin6_addr;
    mreq6.ipv6mr_interface = dev_num_;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, reinterpret_cast<const char *>(&mreq6), sizeof(mreq6)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IPV6_DROP_MEMBERSHIP fail! group ip={}, local ip={}, error={}",
            group_v6_addr->ip_.c_str(), ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V6Addr::AddMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    return true;
}

bool V6Addr::DropMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    return true;
}

bool V6Addr::BlockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    return true;
}

bool V6Addr::UnblockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    return true;
}

bool V6Addr::ParseSa(bool local)
{
    char ip_buf[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET6, &sa6_.sin6_addr, ip_buf, INET6_ADDRSTRLEN);
    ip_ = ip_buf;
    port_ = ntohs(sa6_.sin6_port);

    if (local)
    {
#if defined(_MSC_VER)
        std::vector<uint8_t> pip_adapter_addresses;

        // 获取网络适配器信息
        ULONG buf_size = 0;
        if (ERROR_BUFFER_OVERFLOW == GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &buf_size))
        {
            pip_adapter_addresses.resize(buf_size);
        }

        if (NO_ERROR != GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr,
            reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&pip_adapter_addresses[0]), &buf_size))
        {
            LIB_LOG(lccl::log::Levels::kError, "GetAdaptersAddresses failed");
            return false;
        }

        // 遍历适配器信息
        bool found = false;
        PIP_ADAPTER_ADDRESSES curr_addr = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&pip_adapter_addresses[0]);
        while ((!found) && (curr_addr))
        {
            if (curr_addr->FirstUnicastAddress)
            {
                for (PIP_ADAPTER_UNICAST_ADDRESS uni_addr = curr_addr->FirstUnicastAddress; uni_addr; uni_addr = uni_addr->Next)
                {
                    if (0 == CompareSa(reinterpret_cast<sockaddr *>(uni_addr->Address.lpSockaddr),
                        reinterpret_cast<sockaddr *>(&sa6_)))
                    {
                        dev_ = curr_addr->AdapterName;
                        sockaddr_in6 *sa6 = reinterpret_cast<sockaddr_in6 *>(uni_addr->Address.lpSockaddr);
                        scope_id_ = sa6->sin6_scope_id;
                        sa6_.sin6_scope_id = scope_id_;

                        found = true;
                        break;
                    }
                }
            }
            curr_addr = curr_addr->Next;
        }

        if (!found)
        {
            return false;
        }
#else
        bool found = false;
        struct ifaddrs *ifa = nullptr;
        getifaddrs(&ifa);
        for (struct ifaddrs *node = ifa; node; node = node->ifa_next)
        {
            if ((nullptr != node->ifa_addr) &&
                (0 == CompareSa(reinterpret_cast<sockaddr *>(node->ifa_addr),
                    reinterpret_cast<sockaddr *>(&sa6_))))
            {
                dev_ = (node->ifa_name) ? node->ifa_name : "";
                dev_num_ = if_nametoindex(node->ifa_name);

                sockaddr_in6 *sa6 = reinterpret_cast<sockaddr_in6 *>(node->ifa_addr);
                scope_id_ = sa6->sin6_scope_id;
                sa6_.sin6_scope_id = scope_id_;
                break;
            }
        }
        freeifaddrs(ifa);

        if (!found)
        {
            return false;
        }
#endif
    }

    return true;
}

LCCL_SOCKET_END_NAMESPACE
LCCL_END_NAMESPACE
