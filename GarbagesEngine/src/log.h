#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
namespace GarbagesEngine{
	class GARBAGES_ENGINE_API Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}
// Core log macros
#define GE_CORE_TRACE(...)    ::GarbagesEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...)     ::GarbagesEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...)     ::GarbagesEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...)    ::GarbagesEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_CRITICAL(...)    ::GarbagesEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define GE_TRACE(...)	      ::GarbagesEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GE_INFO(...)	      ::GarbagesEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define GE_WARN(...)	      ::GarbagesEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GE_ERROR(...)	      ::GarbagesEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define GE_CRITICAL(...)	      ::GarbagesEngine::Log::GetClientLogger()->critical(__VA_ARGS__)