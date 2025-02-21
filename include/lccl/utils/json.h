#ifndef LCCL_INCLUDE_UTILS_JSON_H_
#define LCCL_INCLUDE_UTILS_JSON_H_

#include <cstdint>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

inline bool ParseJsonToString(const rapidjson::Value &json_val, std::string &str)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json_val.Accept(writer);
    str = buffer.GetString();
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

inline bool IsJsonChildValid(const rapidjson::Value &json_val, const std::string &key, rapidjson::Type type)
{
    if ((!json_val.IsObject()) || (!json_val.HasMember(key.c_str())) || (json_val[key.c_str()].IsNull()))
    {
        return false;
    }

    const rapidjson::Value &json_child_val = json_val[key.c_str()];
    return (json_child_val.GetType() == type);
}

inline bool GetJsonChildNumber(const rapidjson::Value &json_val, const std::string &key, int &val)
{
    if (!IsJsonChildValid(json_val, key, rapidjson::kNumberType))
    {
        return false;
    }

    val = json_val[key.c_str()].GetInt();
    return true;
}

inline bool GetJsonChildNumber(const rapidjson::Value &json_val, const std::string &key, int64_t &val)
{
    if (!IsJsonChildValid(json_val, key, rapidjson::kNumberType))
    {
        return false;
    }

    val = json_val[key.c_str()].GetInt64();
    return true;
}

inline bool GetJsonChildBool(const rapidjson::Value &json_val, const std::string &key, bool &val)
{
    if ((!IsJsonChildValid(json_val, key, rapidjson::kTrueType)) &&
        (!IsJsonChildValid(json_val, key, rapidjson::kFalseType)))
    {
        return false;
    }

    val = json_val[key.c_str()].GetBool();
    return true;
}

inline bool GetJsonChildString(const rapidjson::Value &json_val, const std::string &key, std::string &val)
{
    if (!IsJsonChildValid(json_val, key, rapidjson::kStringType))
    {
        return false;
    }

    val = json_val[key.c_str()].GetString();
    return true;
}

inline void SetJsonChildNumber(rapidjson::Document &json_doc, const std::string &key, int64_t val)
{
    auto &&allocator = json_doc.GetAllocator();
    json_doc.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val).Move(),
        allocator);
}

inline void SetJsonChildNumber(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, int64_t val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val).Move(),
        allocator);
}

inline void SetJsonChildBool(rapidjson::Document &json_doc, const std::string &key, bool val)
{
    auto &&allocator = json_doc.GetAllocator();
    json_doc.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val).Move(),
        allocator);
}

inline void SetJsonChildBool(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, bool val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val).Move(),
        allocator);
}

inline void SetJsonChildString(rapidjson::Document &json_doc, const std::string &key, const std::string &val)
{
    auto &&allocator = json_doc.GetAllocator();
    json_doc.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val.c_str(), allocator).Move(),
        allocator);
}

inline void SetJsonChildString(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, const std::string &val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value(val.c_str(), allocator).Move(),
        allocator);
}

inline void SetJsonChildObject(rapidjson::Document &json_doc, const std::string &key, rapidjson::Value &json_child_val)
{
    auto &&allocator = json_doc.GetAllocator();
    json_doc.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        json_child_val,
        allocator);
}

inline void SetJsonChildObject(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key, rapidjson::Value &json_child_val)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        json_child_val,
        allocator);
}

inline void SetJsonChildNull(rapidjson::Document &json_doc, const std::string &key)
{
    auto &&allocator = json_doc.GetAllocator();
    json_doc.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value().SetNull().Move(),
        allocator);
}

inline void SetJsonChildNull(rapidjson::Value &json_val, rapidjson::Document::AllocatorType &allocator, const std::string &key)
{
    json_val.AddMember(rapidjson::Value(key.c_str(), allocator).Move(),
        rapidjson::Value().SetNull().Move(),
        allocator);
}

inline void RemoveJsonChild(rapidjson::Document &json_doc, const std::string &key)
{
    if (json_doc.HasMember(key.c_str()))
    {
        json_doc.RemoveMember(key.c_str());
    }
}

inline void RemoveJsonChild(rapidjson::Value &json_val, const std::string &key)
{
    if (json_val.HasMember(key.c_str()))
    {
        json_val.RemoveMember(key.c_str());
    }
}

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_UTILS_JSON_H_
