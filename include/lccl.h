#ifndef LCCL_INCLUDE_LCCL_H_
#define LCCL_INCLUDE_LCCL_H_

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>

#if defined(_MSC_VER)
#if defined(LCCL_API_EXPORT)
#define LCCL_API __declspec(dllexport)
#else
#define LCCL_API __declspec(dllimport)
#endif
#else
#define LCCL_API
#endif

#define LCCL_NAMESPACE_BEGIN namespace lccl {
#define LCCL_NAMESPACE_END }

LCCL_NAMESPACE_BEGIN

LCCL_API const char *GetVersion();

LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_H_
