#pragma once
#ifdef GE_PLATFORM_WINDOWS
#if GE_DYNAMIC_LINK
#ifdef GE_BUILD_DLL
#define GARBAGES_ENGINE_API __declspec(dllexport)
#else
#define GARBAGES_ENGINE_API __declspec(dllimport)
#endif
#else
#define GARBAGES_ENGINE_API
#endif
#else
#error GarbageEngine only supports Windows!
#endif

#ifdef GE_DEBUG
#define GE_ENABLE_ASSERTS
#endif

#ifdef GE_ENABLE_ASSERTS
#define GE_ASSERT(x, ...) { if(!(x)) { GE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define GE_CORE_ASSERT(x, ...) { if(!(x)) { GE_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define GE_ASSERT(x, ...)
#define GE_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define GE_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)