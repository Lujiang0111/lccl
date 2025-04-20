#ifndef LCCL_INCLUDE_LCCL_UTILS_ENUM_CONVERTER_H_
#define LCCL_INCLUDE_LCCL_UTILS_ENUM_CONVERTER_H_

#include "type_converter.h"

LCCL_BEGIN_NAMESPACE

// 一对一使用，正向
template<typename EnumType, typename ToType>
class EnumMapping
{
public:
    void RegisterItem(const EnumType &enum_val, const ToType &to_val)
    {
        to_mapping_.RegisterItem(static_cast<int>(enum_val), to_val);
    }

    const ToType &To(const EnumType &enum_val)
    {
        return to_mapping_.To(static_cast<int>(enum_val));
    }

private:
    ToMapping<int, ToType> to_mapping_;
};

// 一对一使用，双向
template<typename EnumType, typename ToType>
class EnumTwoWayMapping
{
public:
    void RegisterItem(const EnumType &enum_val, const ToType &to_val)
    {
        forward_mapping_.RegisterItem(static_cast<int>(enum_val), to_val);
        reverse_mapping_.RegisterItem(to_val, enum_val);
    }

    const ToType &To(const EnumType &enum_val)
    {
        return forward_mapping_.To(static_cast<int>(enum_val));
    }

    const EnumType &From(const ToType &to_val)
    {
        return reverse_mapping_.To(to_val);
    }

private:
    ToMapping<int, ToType> forward_mapping_;
    ToMapping<ToType, EnumType> reverse_mapping_;
};

// 多对多使用，正向
class EnumConverter
{
public:
    template<typename EnumType, typename ToType>
    void RegisterItem(const EnumType &enum_val, const ToType &to_val)
    {
        type_converter_.RegisterItem<int, ToType>(static_cast<int>(enum_val), to_val);
    }

    template<typename EnumType, typename ToType>
    const ToType &To(const EnumType &enum_val)
    {
        return type_converter_.To<int, ToType>(static_cast<int>(enum_val));
    }

private:
    TypeConverter type_converter_;
};

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_UTILS_ENUM_CONVERTER_H_
