project(GarbagesEngine)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Windows系统动态库生成lib文件命令
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# 创建动态库
add_library(GarbagesEngine SHARED ${SOURCES} )

# 目标链接库
target_include_directories(GarbagesEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 添加 spdlog 子模块
add_subdirectory(vendor/spdlog)

# 包含 spdlog 头文件目录
target_include_directories(GarbagesEngine PUBLIC vendor/spdlog/include)

# 链接 spdlog 库
target_link_libraries(GarbagesEngine PRIVATE spdlog::spdlog)

# 添加 Catch2 子模块
add_subdirectory(vendor/Catch2)

# 链接 Catch2 库
target_link_libraries(GarbagesEngine PRIVATE Catch2::Catch2)

#添加SDL子模块
add_subdirectory(vendor/SDL)

#包含SDL头文件目录
target_include_directories(GarbagesEngine PUBLIC vendor/SDL/include)

# 将 SDL 库链接到你的目标中
target_link_libraries(GarbagesEngine PRIVATE SDL3::SDL3)

#添加Json子模块
add_subdirectory(vendor/Json)

#包含Json头文件目录
target_include_directories(GarbagesEngine PUBLIC vendor/Json/include)

# 将 Json 库链接到你的目标中
target_link_libraries(GarbagesEngine PRIVATE nlohmann_json::nlohmann_json)

# 添加预编译头文件
target_precompile_headers(GarbagesEngine PUBLIC "src/Pch.h")

# 设置宏
# 定义平台相关的宏
if(WIN32)
    set(GE_PLATFORM_WINDOWS 1)
else()
    message(FATAL_ERROR "GarbageEngine only supports Windows!")
endif()

# 定义 'src' 目录的路径
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 定义动态链接相关的宏
option(GE_DYNAMIC_LINK "Use dynamic linking" ON)
option(GE_BUILD_DLL "Build as DLL" OFF)

if(GE_DYNAMIC_LINK)
    set(GARBAGEENGINE_API "__declspec(dllimport)")
    if(GE_BUILD_DLL)
        set(GE_BUILD_DLL 1)
        set(GARBAGEENGINE_API "__declspec(dllexport)")
    endif()
else()
    set(GARBAGEENGINE_API "")
endif()

# 默认宏定义
set(GE_DEBUG 1)  # 例如，开启调试信息

# 传递宏定义到编译器
target_compile_definitions(GarbagesEngine PUBLIC
    GE_PLATFORM_WINDOWS=${GE_PLATFORM_WINDOWS}
    GE_DYNAMIC_LINK=${GE_DYNAMIC_LINK}
    GE_BUILD_DLL=${GE_BUILD_DLL}
    GE_DEBUG=${GE_DEBUG}
)

