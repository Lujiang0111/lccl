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
#include "socket/v4_addr.h"

LCCL_BEGIN_NAMESPACE
LCCL_SOCKET_BEGIN_NAMESPACE

V4Addr::V4Addr() :
    port_(0),
    sa4_{0},
    dev_num_(0)
{

}

V4Addr::~V4Addr()
{

}

bool V4Addr::Init(const char *ip, uint16_t port, bool local)
{
    memset(&sa4_, 0, sizeof(sa4_));
    sa4_.sin_family = AF_INET;
    sa4_.sin_addr.s_addr = inet_addr(ip);
    sa4_.sin_port = htons(port);
    return ParseSa(local);
}

bool V4Addr::Init(const sockaddr *sa, bool local)
{
    memcpy(&sa4_, sa, sizeof(sa4_));
    return ParseSa(local);
}

sockaddr *V4Addr::GetNative()
{
    return reinterpret_cast<sockaddr *>(&sa4_);
}

AddrTypes V4Addr::GetType() const
{
    return AddrTypes::kIpv4;
}

bool V4Addr::IsMulticast() const
{
    return IN_MULTICAST(ntohl(sa4_.sin_addr.s_addr));
}

const char *V4Addr::GetIp() const
{
    return ip_.c_str();
}

uint16_t V4Addr::GetPort() const
{
    return port_;
}

const char *V4Addr::GetDev() const
{
    return dev_.c_str();
}

int V4Addr::Compare(IAddr *rhs)
{
    return CompareSa(GetNative(), rhs->GetNative());
}

bool V4Addr::JoinMulticastGroup(int fd, IAddr *group_addr)
{
    V4Addr *group_v4_addr = dynamic_cast<V4Addr *>(group_addr);
    if (!group_v4_addr)
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr and local addr");
        return false;
    }

    struct ip_mreq mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_v4_addr->sa4_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = sa4_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IP_ADD_MEMBERSHIP fail! group ip={}, local ip={}, error={}",
            group_v4_addr->ip_.c_str(), ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V4Addr::DropMulticastGroup(int fd, IAddr *group_addr)
{
    V4Addr *group_v4_addr = dynamic_cast<V4Addr *>(group_addr);
    if (!group_v4_addr)
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr and local addr");
        return false;
    }

    struct ip_mreq mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_v4_addr->sa4_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = sa4_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IP_DROP_MEMBERSHIP fail! group ip={}, local ip={}, error={}",
            group_v4_addr->ip_.c_str(), ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V4Addr::AddMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    V4Addr *group_v4_addr = dynamic_cast<V4Addr *>(group_addr);
    V4Addr *source_v4_addr = dynamic_cast<V4Addr *>(source_addr);
    if ((!group_v4_addr) || (!source_v4_addr))
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_v4_addr->sa4_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = sa4_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_v4_addr->sa4_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IP_ADD_SOURCE_MEMBERSHIP fail! group ip={}, local ip={}, source ip={}, error={}",
            group_v4_addr->ip_.c_str(), ip_.c_str(), source_v4_addr->ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V4Addr::DropMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    V4Addr *group_v4_addr = dynamic_cast<V4Addr *>(group_addr);
    V4Addr *source_v4_addr = dynamic_cast<V4Addr *>(source_addr);
    if ((!group_v4_addr) || (!source_v4_addr))
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_v4_addr->sa4_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = sa4_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_v4_addr->sa4_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IP_DROP_SOURCE_MEMBERSHIP fail! group ip={}, local ip={}, source ip={}, error={}",
            group_v4_addr->ip_.c_str(), ip_.c_str(), source_v4_addr->ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V4Addr::BlockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    V4Addr *group_v4_addr = dynamic_cast<V4Addr *>(group_addr);
    V4Addr *source_v4_addr = dynamic_cast<V4Addr *>(source_addr);
    if ((!group_v4_addr) || (!source_v4_addr))
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_v4_addr->sa4_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = sa4_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_v4_addr->sa4_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_BLOCK_SOURCE, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IP_BLOCK_SOURCE fail! group ip={}, local ip={}, source ip={}, error={}",
            group_v4_addr->ip_.c_str(), ip_.c_str(), source_v4_addr->ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V4Addr::UnblockMulticastSource(int fd, IAddr *group_addr, IAddr *source_addr)
{
    V4Addr *group_v4_addr = dynamic_cast<V4Addr *>(group_addr);
    V4Addr *source_v4_addr = dynamic_cast<V4Addr *>(source_addr);
    if ((!group_v4_addr) || (!source_v4_addr))
    {
        LIB_LOG(lccl::log::Levels::kError, "Wrong type between group addr, source addr and local addr");
        return false;
    }

    struct ip_mreq_source mreq = { 0 };
    mreq.imr_multiaddr.s_addr = group_v4_addr->sa4_.sin_addr.s_addr;
    mreq.imr_interface.s_addr = sa4_.sin_addr.s_addr;
    mreq.imr_sourceaddr.s_addr = source_v4_addr->sa4_.sin_addr.s_addr;
    if (setsockopt(fd, IPPROTO_IP, IP_UNBLOCK_SOURCE, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) < 0)
    {
        LIB_LOG(lccl::log::Levels::kError, "IP_UNBLOCK_SOURCE fail! group ip={}, local ip={}, source ip={}, error={}",
            group_v4_addr->ip_.c_str(), ip_.c_str(), source_v4_addr->ip_.c_str(), GetLastErrorCode());
        return false;
    }

    return true;
}

bool V4Addr::ParseSa(bool local)
{
    char ip_buf[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &sa4_.sin_addr, ip_buf, INET_ADDRSTRLEN);
    ip_ = ip_buf;
    port_ = ntohs(sa4_.sin_port);

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
                        reinterpret_cast<sockaddr *>(&sa4_)))
                    {
                        dev_ = curr_addr->AdapterName;
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
                    reinterpret_cast<sockaddr *>(&sa4_))))
            {
                dev_ = (node->ifa_name) ? node->ifa_name : "";
                dev_num_ = if_nametoindex(node->ifa_name);
                found = true;
                break;
            }
        }
        freeifaddrs(ifa);
#endif

        if (!found)
        {
            return false;
        }
    }

    return true;
}

LCCL_SOCKET_END_NAMESPACE
LCCL_END_NAMESPACE
