#ifndef LCCL_INCLUDE_LCCL_UTILS_TYPE_CONVERTER_H_
#define LCCL_INCLUDE_LCCL_UTILS_TYPE_CONVERTER_H_

#include <memory>
#include <typeindex>
#include <unordered_map>
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

// 一对一使用
template<typename FromType, typename ToType>
class ToMapping
{
public:
    void RegisterItem(const FromType &from_val, const ToType &to_val)
    {
        from_map_.emplace(from_val, to_val);
    }

    void RegisterMap(const std::unordered_map<FromType, ToType> &map)
    {
        from_map_.insert(map.begin(), map.end());
    }

    void RegisterReverseMapp(const std::unordered_map<ToType, FromType> &map)
    {
        for (const auto &pair : map)
        {
            from_map_.emplace(pair.second, pair.first);
        }
    }

    const ToType &To(const FromType &from_val)
    {
        auto it = from_map_.find(from_val);
        if (from_map_.end() == it)
        {
            throw std::out_of_range("Mapping not found.");
        }
        return it->second;
    }

private:
    std::unordered_map<FromType, ToType> from_map_;
};

// 一对多使用
template<typename FromType, typename ToType>
class FromMapping
{
public:
    void RegisterItem(const FromType &from_val, const ToType &to_val)
    {
        auto &ptr = GetOrCreateMapping();
        ptr.RegisterItem(from_val, to_val);
    }

    void RegisterMap(const std::unordered_map<FromType, ToType> &map)
    {
        auto &ptr = GetOrCreateMapping();
        ptr.RegisterMap(map);
    }

    const ToType &To(const FromType &from_val)
    {
        return GetMapping().To(from_val);
    }

private:
    ToMapping<FromType, ToType> &GetOrCreateMapping()
    {
        std::type_index key = std::type_index(typeid(ToType));
        auto it = registry_.find(key);
        if (registry_.end() == it)
        {
            registry_[key] = std::make_shared<ToMapping<FromType, ToType>>();
            it = registry_.find(key);
        }
        return *std::static_pointer_cast<ToMapping<FromType, ToType>>(it->second);
    }

    ToMapping<FromType, ToType> &GetMapping()
    {
        std::type_index key = std::type_index(typeid(ToType));
        auto it = registry_.find(key);
        if (registry_.end() == it)
        {
            throw std::out_of_range("Mapping not found.");
        }
        return *std::static_pointer_cast<ToMapping<FromType, ToType>>(it->second);
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> registry_;
};

// 多对多使用
class TypeConverter
{
public:
    template<typename FromType, typename ToType>
    void RegisterItem(const FromType &from_val, const ToType &to_val)
    {
        auto &ptr = GetOrCreateMapping<FromType, ToType>();
        ptr.RegisterItem(from_val, to_val);
    }

    template<typename FromType, typename ToType>
    void RegisterMap(const std::unordered_map<FromType, ToType> &map)
    {
        auto &ptr = GetOrCreateMapping<FromType, ToType>();
        ptr.RegisterMap(map);
    }

    template<typename FromType, typename ToType>
    const ToType &To(const FromType &from_val)
    {
        return GetMapping<FromType, ToType>().To(from_val);
    }

private:
    template<typename FromType, typename ToType>
    FromMapping<FromType, ToType> &GetOrCreateMapping()
    {
        std::type_index key = std::type_index(typeid(FromType));
        auto it = registry_.find(key);
        if (registry_.end() == it)
        {
            registry_[key] = std::make_shared<FromMapping<FromType, ToType>>();
            it = registry_.find(key);
        }
        return *std::static_pointer_cast<FromMapping<FromType, ToType>>(it->second);
    }

    template<typename FromType, typename ToType>
    FromMapping<FromType, ToType> &GetMapping()
    {
        std::type_index key = std::type_index(typeid(FromType));
        auto it = registry_.find(key);
        if (registry_.end() == it)
        {
            throw std::out_of_range("Mapping not found.");
        }
        return *std::static_pointer_cast<FromMapping<FromType, ToType>>(it->second);
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> registry_;
};

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_UTILS_TYPE_CONVERTER_H_
