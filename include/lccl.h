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

#define LCCL_BEGIN_NAMESPACE namespace lccl {
#define LCCL_END_NAMESPACE }

LCCL_BEGIN_NAMESPACE

LCCL_API const char *GetVersion();

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_H_
