#ifndef LCCL_INCLUDE_LCCL_OSS_JSON_H_
#define LCCL_INCLUDE_LCCL_OSS_JSON_H_

#include <string>
#include "lccl/oss/rapidjson/document.h"
#include "lccl/oss/rapidjson/stringbuffer.h"
#include "lccl/oss/rapidjson/writer.h"
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

inline bool ParseJsonToString(const rapidjson::Value &json_val, std::string &str)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (!json_val.Accept(writer))
    {
        return false;
    }

    str.assign(buffer.GetString(), buffer.GetSize());
    return true;
}

inline bool ParseStringToJson(rapidjson::Document &json_doc, const std::string &str)
{
    rapidjson::Document parsed_doc;
    if (parsed_doc.Parse(str.c_str()).HasParseError())
    {
        return false;
    }

    json_doc.Swap(parsed_doc);
    return true;
}

inline bool ParseStringToJson(rapidjson::Value &json_val, const std::string &str, rapidjson::Document::AllocatorType &allocator)
{
    rapidjson::Document json_doc;
    if (!ParseStringToJson(json_doc, str))
    {
        return false;
    }

    json_val.CopyFrom(json_doc, allocator);
    return true;
}

inline rapidjson::Value::ConstMemberIterator::Pointer GetJsonChildPointer(const rapidjson::Value &json_val, const std::string &key)
{
    if (!json_val.IsObject())
    {
        return nullptr;
    }

    auto &&iter = json_val.FindMember(key.c_str());
    if (json_val.MemberEnd() == iter)
    {
        return nullptr;
    }
    return iter.operator->();
}

inline bool IsJsonChildValid(const rapidjson::Value &json_val, const std::string &key, rapidjson::Type type)
{
    rapidjson::Value::ConstMemberIterator::Pointer val_pointer = GetJsonChildPointer(json_val, key);
    if (!val_pointer)
    {
        return false;
    }

    return (val_pointer->value.GetType() == type);
}

template<typename T>
inline typename std::enable_if<std::is_signed<T>::value, bool>::type
GetJsonChild(const rapidjson::Value &json_val, const std::string &key, T &val)
{
    rapidjson::Value::ConstMemberIterator::Pointer val_pointer = GetJsonChildPointer(json_val, key);
    if (!val_pointer)
    {
        return false;
    }
    const rapidjson::Value &child_val = val_pointer->value;

    if (!child_val.IsInt64())
    {
        return false;
    }

    val = static_cast<T>(val_pointer->value.GetInt64());
    return true;
}

template<typename T>
inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type
GetJsonChild(const rapidjson::Value &json_val, const std::string &key, T &val)
{
    rapidjson::Value::ConstMemberIterator::Pointer val_pointer = GetJsonChildPointer(json_val, key);
    if (!val_pointer)
    {
        return false;
    }
    const rapidjson::Value &child_val = val_pointer->value;

    if (!child_val.IsUint64())
    {
        return false;
    }

    val = static_cast<T>(child_val.GetUint64());
    return true;
}

inline bool GetJsonChild(const rapidjson::Value &json_val, const std::string &key, bool &val)
{
    rapidjson::Value::ConstMemberIterator::Pointer val_pointer = GetJsonChildPointer(json_val, key);
    if (!val_pointer)
    {
        return false;
    }
    const rapidjson::Value &child_val = val_pointer->value;

    if (!child_val.IsBool())
    {
        return false;
    }

    val = child_val.GetBool();
    return true;
}

inline bool GetJsonChild(const rapidjson::Value &json_val, const std::string &key, std::string &val)
{
    rapidjson::Value::ConstMemberIterator::Pointer val_pointer = GetJsonChildPointer(json_val, key);
    if (!val_pointer)
    {
        return false;
    }
    const rapidjson::Value &child_val = val_pointer->value;

    if (!child_val.IsString())
    {
        return false;
    }

    val = child_val.GetString();
    return true;
}

template<typename T>
inline typename std::enable_if<std::is_signed<T>::value, void>::type
SetJsonChild(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, T val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(static_cast<int64_t>(val)).Move(),
        allocator);
}

template<typename T>
inline typename std::enable_if<std::is_unsigned<T>::value, void>::type
SetJsonChild(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, T val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(static_cast<uint64_t>(val)).Move(),
        allocator);
}

inline void SetJsonChild(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, bool val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val).Move(),
        allocator);
}

inline void SetJsonChild(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, const std::string &val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val.c_str(), allocator).Move(),
        allocator);
}

inline void SetJsonChild(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, rapidjson::Value &json_child_val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        json_child_val,
        allocator);
}

inline void SetJsonChildNull(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value().SetNull().Move(),
        allocator);
}

inline void RemoveJsonChild(rapidjson::Value &json_val, const std::string &key)
{
    if (json_val.HasMember(key.c_str()))
    {
        json_val.RemoveMember(key.c_str());
    }
}

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_OSS_JSON_H_
