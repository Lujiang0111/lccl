#ifndef LCCL_INCLUDE_LCCL_UTILS_ENUM_UTILS_H_
#define LCCL_INCLUDE_LCCL_UTILS_ENUM_UTILS_H_

#include <type_traits>
#include <unordered_map>
#include "lccl.h"

LCCL_NAMESPACE_BEGIN

// 基础映射类
template<typename SrcType, typename DstType>
class TypeMapping
{
public:
    void RegisterItem(const SrcType &src_val, const DstType &dst_val)
    {
        map_.emplace(src_val, dst_val);
    }

    void RegisterMap(const std::unordered_map<SrcType, DstType> &map)
    {
        map_.insert(map.begin(), map.end());
    }

    const DstType &To(const SrcType &src_val)
    {
        auto &&it = map_.find(src_val);
        if (map_.end() == it)
        {
            throw std::out_of_range("Mapping not found.");
        }
        return it->second;
    }

private:
    std::unordered_map<SrcType, DstType> map_;
};

// 单向使用
template<typename EnumType, typename ToType>
class EnumMapping
{
public:
    void RegisterItem(const EnumType &enum_val, const ToType &to_val)
    {
        map_.RegisterItem(static_cast<typename std::underlying_type<EnumType>::type>(enum_val), to_val);
    }

    const ToType &To(const EnumType &enum_val)
    {
        return map_.To(static_cast<typename std::underlying_type<EnumType>::type>(enum_val));
    }

private:
    TypeMapping<typename std::underlying_type<EnumType>::type, ToType> map_;
};

// 双向使用
template<typename EnumType, typename DstType>
class EnumTwoWayMapping
{
public:
    void RegisterItem(const EnumType &enum_val, const DstType &dst_val)
    {
        map_.RegisterItem(static_cast<typename std::underlying_type<EnumType>::type>(enum_val), dst_val);
        reverse_map_.RegisterItem(dst_val, enum_val);
    }

    const DstType &To(const EnumType &enum_val)
    {
        return map_.To(static_cast<typename std::underlying_type<EnumType>::type>(enum_val));
    }

    const DstType &From(const DstType &dst_val)
    {
        return reverse_map_.To(dst_val);
    }

private:
    TypeMapping<typename std::underlying_type<EnumType>::type, DstType> map_;
    TypeMapping<DstType, EnumType> reverse_map_;
};

LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_UTILS_ENUM_UTILS_H_
